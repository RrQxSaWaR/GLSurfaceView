#include <jni.h>
#include <pthread.h>
#include <unistd.h>

#include "Globals.h"
#include "Injector.h"
#include "Renderer.h"

inline void* StartupThread(void*) {
    JNIEnv* env;
    if (g_JVM->AttachCurrentThread(&env, nullptr) != JNI_OK) {
        return nullptr;
    }

    int attempts = 0;
    while (attempts < 100) {
        jobject activity = GetTargetActivity(env);
        if (activity) {
            g_ActivityGlobalRef = env->NewGlobalRef(activity);
            env->DeleteLocalRef(activity);

            jobject sv = LoadDexAndInject(env, g_ActivityGlobalRef);
            if (sv) {
                g_SurfaceViewGlobalRef = env->NewGlobalRef(sv);
                env->DeleteLocalRef(sv);

                pthread_t renderThread;
                if (pthread_create(&renderThread, nullptr, RenderThread, nullptr) == 0) {
                    pthread_detach(renderThread);
                }
                break;
            }
            break;
        }
        usleep(100000);
        attempts++;
    }

    g_JVM->DetachCurrentThread();
    return nullptr;
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved) {
    g_JVM = vm;
    
    pthread_t thread;
    if (pthread_create(&thread, nullptr, StartupThread, nullptr) == 0) {
        pthread_detach(thread);
    }
    
    return JNI_VERSION_1_6;
}
