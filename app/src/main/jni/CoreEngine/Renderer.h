#pragma once
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES3/gl3.h>
#include <android/native_window.h>
#include <android/native_window_jni.h>
#include <unistd.h>
#include <vector>
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include "Globals.h"
#include "Menu.h"

#ifndef EGL_OPENGL_ES3_BIT_KHR
#define EGL_OPENGL_ES3_BIT_KHR 0x00000040
#endif

#ifndef EGL_OPENGL_ES3_BIT
#define EGL_OPENGL_ES3_BIT EGL_OPENGL_ES3_BIT_KHR
#endif

#ifndef WINDOW_FORMAT_RGBA_8888
#define WINDOW_FORMAT_RGBA_8888 1
#endif

inline void* RenderThread(void*) {
    JNIEnv* env;
    if (g_JVM->AttachCurrentThread(&env, nullptr) != JNI_OK) return nullptr;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.IniFilename = nullptr;
    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;

    io.FontGlobalScale = 1.9f; 
    ImGuiStyle *style = &ImGui::GetStyle();
    style->ScaleAllSizes(2.3f); 
    ImGui::StyleColorsDark();
    
    style->Alpha = 1.0f;
    style->WindowTitleAlign = ImVec2(0.5f, 0.5f); 
    style->WindowRounding = 0.0f;
    style->ChildRounding = 0.0f;
    style->FrameRounding = 0.0f;
    style->PopupRounding = 0.0f;
    style->GrabRounding = 0.0f;
    style->TabRounding = 0.0f;

    EGLDisplay display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    if (display == EGL_NO_DISPLAY || !eglInitialize(display, nullptr, nullptr)) {
        ImGui::DestroyContext();
        g_JVM->DetachCurrentThread();
        return nullptr;
    }

    const EGLint attribs[] = {
        EGL_SURFACE_TYPE, EGL_WINDOW_BIT,
        EGL_RENDERABLE_TYPE, EGL_OPENGL_ES3_BIT,
        EGL_RED_SIZE, 8, EGL_GREEN_SIZE, 8, EGL_BLUE_SIZE, 8, EGL_ALPHA_SIZE, 8,
        EGL_DEPTH_SIZE, 16, EGL_NONE
    };

    EGLint numConfigs = 0;
    eglChooseConfig(display, attribs, nullptr, 0, &numConfigs);
    if (numConfigs <= 0) {
        eglTerminate(display); ImGui::DestroyContext(); g_JVM->DetachCurrentThread();
        return nullptr;
    }

    std::vector<EGLConfig> configs(numConfigs);
    eglChooseConfig(display, attribs, configs.data(), numConfigs, &numConfigs);

    EGLConfig chosenConfig = nullptr;
    for (int i = 0; i < numConfigs; ++i) {
        EGLint r, g, b, a;
        eglGetConfigAttrib(display, configs[i], EGL_RED_SIZE, &r);
        eglGetConfigAttrib(display, configs[i], EGL_GREEN_SIZE, &g);
        eglGetConfigAttrib(display, configs[i], EGL_BLUE_SIZE, &b);
        eglGetConfigAttrib(display, configs[i], EGL_ALPHA_SIZE, &a);
        if (r == 8 && g == 8 && b == 8 && a == 8) { chosenConfig = configs[i]; break; }
    }
    if (!chosenConfig) chosenConfig = configs[0];

    const EGLint contextAttribs[] = { EGL_CONTEXT_CLIENT_VERSION, 3, EGL_NONE };
    EGLContext context = eglCreateContext(display, chosenConfig, EGL_NO_CONTEXT, contextAttribs);
    if (context == EGL_NO_CONTEXT) {
        eglTerminate(display); ImGui::DestroyContext(); g_JVM->DetachCurrentThread();
        return nullptr;
    }

    bool imguiBackendReady = false;

    while (g_RenderThreadRunning) {
        if (!g_SurfaceViewGlobalRef) { usleep(50000); continue; }

        jclass svClass = env->GetObjectClass(g_SurfaceViewGlobalRef);
        jmethodID getHolder = env->GetMethodID(svClass, "getHolder", "()Landroid/view/SurfaceHolder;");
        jobject holder = getHolder ? env->CallObjectMethod(g_SurfaceViewGlobalRef, getHolder) : nullptr;
        env->DeleteLocalRef(svClass);

        if (!holder || env->ExceptionCheck()) { env->ExceptionClear(); usleep(50000); continue; }

        jclass holderClass = env->GetObjectClass(holder);
        jmethodID getSurface = env->GetMethodID(holderClass, "getSurface", "()Landroid/view/Surface;");
        jobject surface = getSurface ? env->CallObjectMethod(holder, getSurface) : nullptr;
        env->DeleteLocalRef(holder); env->DeleteLocalRef(holderClass);

        if (!surface || env->ExceptionCheck()) { env->ExceptionClear(); usleep(50000); continue; }

        jclass surfaceClass = env->GetObjectClass(surface);
        jmethodID isValid = env->GetMethodID(surfaceClass, "isValid", "()Z");
        jboolean valid = isValid ? env->CallBooleanMethod(surface, isValid) : JNI_FALSE;

        if (env->ExceptionCheck() || !valid) { env->ExceptionClear(); env->DeleteLocalRef(surface); env->DeleteLocalRef(surfaceClass); usleep(50000); continue; }

        ANativeWindow* window = ANativeWindow_fromSurface(env, surface);
        env->DeleteLocalRef(surface); env->DeleteLocalRef(surfaceClass);

        if (!window) { usleep(100000); continue; }

        int32_t winWidth = ANativeWindow_getWidth(window);
        int32_t winHeight = ANativeWindow_getHeight(window);
        
        if (winWidth <= 0 || winHeight <= 0) { ANativeWindow_release(window); usleep(50000); continue; }

        ANativeWindow_setBuffersGeometry(window, winWidth, winHeight, WINDOW_FORMAT_RGBA_8888);

        EGLSurface eglSurface = eglCreateWindowSurface(display, chosenConfig, window, nullptr);
        if (eglSurface == EGL_NO_SURFACE) { ANativeWindow_release(window); usleep(100000); continue; }

        if (!eglMakeCurrent(display, eglSurface, eglSurface, context)) {
            eglDestroySurface(display, eglSurface); ANativeWindow_release(window); usleep(100000); continue;
        }

        eglSwapInterval(display, 1);

        if (!imguiBackendReady) {
            if (!ImGui_ImplOpenGL3_Init("#version 300 es")) {
                eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
                eglDestroySurface(display, eglSurface); ANativeWindow_release(window);
                break;
            }
            imguiBackendReady = true;
        }

        bool surfaceLost = false;

        while (!surfaceLost && g_RenderThreadRunning) {
            EGLint width = 0, height = 0;
            eglQuerySurface(display, eglSurface, EGL_WIDTH, &width);
            eglQuerySurface(display, eglSurface, EGL_HEIGHT, &height);

            if (width <= 0 || height <= 0) { usleep(32000); continue; }

            glViewport(0, 0, width, height);
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

            {
                std::lock_guard<std::mutex> lock(g_ImGuiMutex);
                io.DisplaySize = ImVec2((float)width, (float)height);
                ImGui_ImplOpenGL3_NewFrame();
                ImGui::NewFrame();
                RenderImGuiContent();
                ImGui::Render();
                ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            }

            if (!eglSwapBuffers(display, eglSurface)) {
                EGLint err = eglGetError();
                if (err == EGL_BAD_SURFACE || err == EGL_CONTEXT_LOST || err == EGL_BAD_NATIVE_WINDOW) { surfaceLost = true; }
            }
            usleep(16000);
        }

        eglMakeCurrent(display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        eglDestroySurface(display, eglSurface);
        ANativeWindow_release(window);

        if (g_RenderThreadRunning) usleep(100000);
    }

    if (imguiBackendReady) ImGui_ImplOpenGL3_Shutdown();
    eglDestroyContext(display, context);
    eglTerminate(display);
    ImGui::DestroyContext();
    g_JVM->DetachCurrentThread();
    return nullptr;
}
