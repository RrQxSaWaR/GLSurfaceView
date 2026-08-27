#pragma once
#include <jni.h>
#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include "Globals.h"
#include "Menu.h"

inline bool IsTouchInsideImGui(float x, float y) {
    ImGuiContext* g = ImGui::GetCurrentContext();
    if (!g) return false;

    for (int i = g->Windows.Size - 1; i >= 0; i--) {
        ImGuiWindow* window = g->Windows[i];
        if (!window || !window->Active || window->Hidden || (window->Flags & ImGuiWindowFlags_NoInputs)) {
            continue;
        }

        ImRect bb = window->Rect();
        if (bb.Contains(ImVec2(x, y))) {
            return true;
        }
    }
    return false;
}

extern "C" JNIEXPORT jboolean JNICALL
Java_com_rrq_overlay_NativeHelper_onNativeTouch(JNIEnv* env, jclass clazz, jfloat x, jfloat y, jint action) {
    std::lock_guard<std::mutex> lock(g_ImGuiMutex);
    if (!ImGui::GetCurrentContext()) return JNI_FALSE;

    ImGuiIO& io = ImGui::GetIO();
    switch (action) {
        case 0: 
            if (IsTouchInsideImGui(x, y)) {
                io.MousePos = ImVec2(x, y);
                io.MouseDown[0] = true;
                return JNI_TRUE; 
            }
            return JNI_FALSE;
        case 1: 
            io.MousePos = ImVec2(x, y);
            io.MouseDown[0] = false;
            return JNI_TRUE;
        case 2: 
            io.MousePos = ImVec2(x, y);
            return JNI_TRUE;
        case 3: 
            io.MouseDown[0] = false;
            return JNI_TRUE;
    }
    return JNI_FALSE;
}

extern "C" JNIEXPORT void JNICALL
Java_com_rrq_overlay_NativeHelper_onNativeScroll(JNIEnv* env, jclass clazz, jfloat scrollY) {
    std::lock_guard<std::mutex> lock(g_ImGuiMutex);
    if (!ImGui::GetCurrentContext()) return;
    ImGui::GetIO().MouseWheel += (scrollY * 0.05f);
}

inline const JNINativeMethod g_NativeMethods[] = {
    { const_cast<char*>("onNativeTouch"), const_cast<char*>("(FFI)Z"), reinterpret_cast<void*>(&Java_com_rrq_overlay_NativeHelper_onNativeTouch) },
    { const_cast<char*>("onNativeScroll"), const_cast<char*>("(F)V"), reinterpret_cast<void*>(&Java_com_rrq_overlay_NativeHelper_onNativeScroll) }
};

inline jobject GetTargetActivity(JNIEnv* env) {
    env->ExceptionClear();

    // =========================================================================
    // 1. Unreal Engine (UE4 & UE5) Entry Points
    // =========================================================================
    struct UEActivityTarget {
        const char* className;
        const char* fieldName;
        const char* fieldSig;
    };

    const UEActivityTarget ueTargets[] = {
        // Standard UE4
        { "com/epicgames/ue4/GameActivity", "_activity", "Lcom/epicgames/ue4/GameActivity;" },
        { "com/epicgames/ue4/GameActivity", "_activity", "Landroid/app/Activity;" },
        { "com/epicgames/ue4/SplashActivity", "_activity", "Landroid/app/Activity;" },
        // Modern UE5
        { "com/epicgames/unreal/GameActivity", "_activity", "Lcom/epicgames/unreal/GameActivity;" },
        { "com/epicgames/unreal/GameActivity", "_activity", "Landroid/app/Activity;" },
        { "com/epicgames/unreal/SplashActivity", "_activity", "Landroid/app/Activity;" }
    };

    for (const auto& target : ueTargets) {
        jclass ueClass = env->FindClass(target.className);
        if (ueClass && !env->ExceptionCheck()) {
            jfieldID fieldId = env->GetStaticFieldID(ueClass, target.fieldName, target.fieldSig);
            if (fieldId && !env->ExceptionCheck()) {
                jobject activity = env->GetStaticObjectField(ueClass, fieldId);
                if (activity && !env->ExceptionCheck()) {
                    env->DeleteLocalRef(ueClass);
                    return activity;
                }
            }
            env->ExceptionClear();
            env->DeleteLocalRef(ueClass);
        } else {
            env->ExceptionClear();
        }
    }

    // =========================================================================
    // 2. Unity (IL2CPP, Mono & Custom Game Wrappers) Entry Points
    // =========================================================================
    const char* unityClasses[] = {
        "com/unity3d/player/UnityPlayer",
        "com/unity3d/player/UnityPlayerActivity",
        "com/unity3d/player/UnityPlayerNativeActivity",
        "com/unity3d/player/UnityPlayerForActivityOrService",
        "com/scopely/unity/ScopelyUnityActivity",
		"com.dong.multirun.Ovbk$RA",
		"com.dong.multiui.home.MultiHomeActivity"
    };

    for (const char* uClass : unityClasses) {
        jclass unityCls = env->FindClass(uClass);
        if (unityCls && !env->ExceptionCheck()) {
            // Check for standard currentActivity field
            jfieldID currentActivityId = env->GetStaticFieldID(unityCls, "currentActivity", "Landroid/app/Activity;");
            if (currentActivityId && !env->ExceptionCheck()) {
                jobject activity = env->GetStaticObjectField(unityCls, currentActivityId);
                if (activity && !env->ExceptionCheck()) {
                    env->DeleteLocalRef(unityCls);
                    return activity;
                }
            }
            env->ExceptionClear();

            // Check for custom _activity field
            jfieldID customActivityId = env->GetStaticFieldID(unityCls, "_activity", "Landroid/app/Activity;");
            if (customActivityId && !env->ExceptionCheck()) {
                jobject activity = env->GetStaticObjectField(unityCls, customActivityId);
                if (activity && !env->ExceptionCheck()) {
                    env->DeleteLocalRef(unityCls);
                    return activity;
                }
            }
            env->ExceptionClear();
            env->DeleteLocalRef(unityCls);
        } else {
            env->ExceptionClear();
        }
    }

    // =========================================================================
    // 3. Custom Engines & Dedicated Game Classes
    // =========================================================================
    const char* customClasses[] = {
        // --- Gameloft / Asphalt Series ---
        "com/gameloft/android/ANMP/GloftA8HM/MainActivity", // Asphalt 8: Airborne
        "com/gameloft/android/ANMP/GloftA8HM/GLActivity",
        "com/gameloft/android/ANMP/GloftA9HM/MainActivity", // Asphalt 9: Legends / Legends Unite
        "com/gameloft/android/ANMP/GloftA9HM/GLActivity",
        "com/gameloft/android/ANMP/GloftAGHM/MainActivity", // Asphalt Nitro
        "com/gameloft/android/ANMP/GloftMTHM/MainActivity", // Asphalt Xtreme
        "com/gameloft/android/ANMP/GloftAOHM/MainActivity", // Asphalt Overdrive

        // --- Other Custom Engines ---
        "com/ea/gp/fifamobile/FifaMainActivity",           // FIFA Mobile
        "com/appsomniacs/da2/DA2Activity",                 // Mini Militia (DA2)
        "org/cocos2dx/lib/Cocos2dxActivity"                // Cocos2d-x Base
    };

    // Static instance fields across Gameloft, EA, Appsomniacs, and Cocos
    const char* commonStaticFields[] = {
        "currentActivity", "_activity", "mInstance", "instance", 
        "sInstance", "m_instance", "mActivity", "sContext"
    };

    for (const char* customClsName : customClasses) {
        jclass customCls = env->FindClass(customClsName);
        if (customCls && !env->ExceptionCheck()) {
            for (const char* fieldName : commonStaticFields) {
                // Try as Activity first
                jfieldID fieldId = env->GetStaticFieldID(customCls, fieldName, "Landroid/app/Activity;");
                
                // Fallback as generic Context if not declared directly as Activity
                if (!fieldId || env->ExceptionCheck()) {
                    env->ExceptionClear();
                    fieldId = env->GetStaticFieldID(customCls, fieldName, "Landroid/content/Context;");
                }

                if (fieldId && !env->ExceptionCheck()) {
                    jobject activity = env->GetStaticObjectField(customCls, fieldId);
                    if (activity && !env->ExceptionCheck()) {
                        env->DeleteLocalRef(customCls);
                        return activity;
                    }
                }
                env->ExceptionClear();
            }
            env->DeleteLocalRef(customCls);
        } else {
            env->ExceptionClear();
        }
    }

    // =========================================================================
    // 4. Fallback: Generic ActivityThread Traversal (Catches All Remaining)
    // =========================================================================
    jclass activityThread = env->FindClass("android/app/ActivityThread");
    if (!activityThread || env->ExceptionCheck()) {
        env->ExceptionClear();
        return nullptr;
    }

    jmethodID currentActivityThread = env->GetStaticMethodID(activityThread, "currentActivityThread", "()Landroid/app/ActivityThread;");
    jobject at = currentActivityThread ? env->CallStaticObjectMethod(activityThread, currentActivityThread) : nullptr;
    if (!at || env->ExceptionCheck()) {
        env->ExceptionClear();
        env->DeleteLocalRef(activityThread);
        return nullptr;
    }

    jfieldID mActivitiesId = env->GetFieldID(activityThread, "mActivities", "Landroid/util/ArrayMap;");
    jobject mActivities = mActivitiesId ? env->GetObjectField(at, mActivitiesId) : nullptr;
    env->DeleteLocalRef(at);
    env->DeleteLocalRef(activityThread);
    if (!mActivities || env->ExceptionCheck()) {
        env->ExceptionClear();
        return nullptr;
    }

    jclass arrayMapClass = env->FindClass("android/util/ArrayMap");
    jmethodID valueAtMethod = env->GetMethodID(arrayMapClass, "valueAt", "(I)Ljava/lang/Object;");
    jmethodID sizeMethod = env->GetMethodID(arrayMapClass, "size", "()I");

    jobject targetActivity = nullptr;
    if (valueAtMethod && sizeMethod) {
        jint size = env->CallIntMethod(mActivities, sizeMethod);
        for (int i = 0; i < size; ++i) {
            jobject record = env->CallObjectMethod(mActivities, valueAtMethod, i);
            if (record && !env->ExceptionCheck()) {
                jclass recordClass = env->GetObjectClass(record);
                
                // Inspect ActivityClientRecord.activity
                jfieldID activityField = env->GetFieldID(recordClass, "activity", "Landroid/app/Activity;");
                if (activityField) {
                    targetActivity = env->GetObjectField(record, activityField);
                }
                
                // Verify the activity is not currently paused/stopped
                if (targetActivity) {
                    jfieldID pausedField = env->GetFieldID(recordClass, "paused", "Z");
                    if (pausedField) {
                        jboolean isPaused = env->GetBooleanField(record, pausedField);
                        if (isPaused) {
                            env->DeleteLocalRef(targetActivity);
                            targetActivity = nullptr;
                        }
                    }
                }

                env->DeleteLocalRef(recordClass);
                env->DeleteLocalRef(record);
                if (targetActivity) break;
            }
            env->ExceptionClear();
        }
    }

    env->DeleteLocalRef(arrayMapClass);
    env->DeleteLocalRef(mActivities);
    env->ExceptionClear();
    return targetActivity;
}

inline jobject LoadDexAndInject(JNIEnv* env, jobject activity) {
    jclass activityClass = env->GetObjectClass(activity);
    jmethodID getAppContext = env->GetMethodID(activityClass, "getApplicationContext", "()Landroid/content/Context;");
    jobject appContext = env->CallObjectMethod(activity, getAppContext);
    env->DeleteLocalRef(activityClass);

    if (!appContext || env->ExceptionCheck()) { env->ExceptionClear(); return nullptr; }

    jclass contextClass = env->GetObjectClass(appContext);
    jmethodID getCL = env->GetMethodID(contextClass, "getClassLoader", "()Ljava/lang/ClassLoader;");
    jobject parentLoader = env->CallObjectMethod(appContext, getCL);
    jobject dexClassLoader = nullptr;

    jclass inMemoryLoaderClass = env->FindClass("dalvik/system/InMemoryDexClassLoader");
    if (inMemoryLoaderClass && !env->ExceptionCheck()) {
        jmethodID inMemoryInit = env->GetMethodID(inMemoryLoaderClass, "<init>", "(Ljava/nio/ByteBuffer;Ljava/lang/ClassLoader;)V");
        if (inMemoryInit && RrQ_Size > 0) {
            void* dexBuffer = malloc(RrQ_Size);
            if (dexBuffer) {
                memcpy(dexBuffer, RrQ_Bytes, RrQ_Size);
                jobject byteBuffer = env->NewDirectByteBuffer(dexBuffer, (jlong)RrQ_Size);
                if (byteBuffer) {
                    dexClassLoader = env->NewObject(inMemoryLoaderClass, inMemoryInit, byteBuffer, parentLoader);
                    if (env->ExceptionCheck()) { env->ExceptionClear(); dexClassLoader = nullptr; }
                    env->DeleteLocalRef(byteBuffer);
                }
                free(dexBuffer);
            }
        }
        env->DeleteLocalRef(inMemoryLoaderClass);
    } else {
        env->ExceptionClear();
    }

    if (!dexClassLoader && RrQ_Size > 0) {
        jmethodID getCacheDir = env->GetMethodID(contextClass, "getCodeCacheDir", "()Ljava/io/File;");
        if (!getCacheDir) {
            env->ExceptionClear();
            getCacheDir = env->GetMethodID(contextClass, "getCacheDir", "()Ljava/io/File;");
        }
        jobject cacheDir = env->CallObjectMethod(appContext, getCacheDir);
        if (cacheDir && !env->ExceptionCheck()) {
            jclass fileClass = env->FindClass("java/io/File");
            jmethodID getAbsolutePath = env->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
            jstring cacheDirPathStr = (jstring)env->CallObjectMethod(cacheDir, getAbsolutePath);
            jmethodID fileInit = env->GetMethodID(fileClass, "<init>", "(Ljava/io/File;Ljava/lang/String;)V");
            jstring dexFileName = env->NewStringUTF("overlay_runtime.dex");
            jobject dexFile = env->NewObject(fileClass, fileInit, cacheDir, dexFileName);
            env->DeleteLocalRef(dexFileName);

            jclass fosClass = env->FindClass("java/io/FileOutputStream");
            jmethodID fosInit = env->GetMethodID(fosClass, "<init>", "(Ljava/io/File;)V");
            jobject fos = env->NewObject(fosClass, fosInit, dexFile);

            if (fos && !env->ExceptionCheck()) {
                jbyteArray byteArray = env->NewByteArray(RrQ_Size);
                env->SetByteArrayRegion(byteArray, 0, RrQ_Size, (const jbyte*)RrQ_Bytes);
                jmethodID writeMethod = env->GetMethodID(fosClass, "write", "([B)V");
                env->CallVoidMethod(fos, writeMethod, byteArray);
                jmethodID closeMethod = env->GetMethodID(fosClass, "close", "()V");
                env->CallVoidMethod(fos, closeMethod);
                env->DeleteLocalRef(byteArray);
                env->DeleteLocalRef(fos);

                jstring dexPathStr = (jstring)env->CallObjectMethod(dexFile, getAbsolutePath);
                jclass dclClass = env->FindClass("dalvik/system/DexClassLoader");
                jmethodID dclInit = env->GetMethodID(dclClass, "<init>", "(Ljava/lang/String;Ljava/lang/String;Ljava/lang/String;Ljava/lang/ClassLoader;)V");
                dexClassLoader = env->NewObject(dclClass, dclInit, dexPathStr, cacheDirPathStr, nullptr, parentLoader);
                if (env->ExceptionCheck()) { env->ExceptionClear(); dexClassLoader = nullptr; }
                env->DeleteLocalRef(dexPathStr);
                env->DeleteLocalRef(dclClass);
            } else {
                env->ExceptionClear();
            }
            env->DeleteLocalRef(fosClass);
            env->DeleteLocalRef(dexFile);
            env->DeleteLocalRef(cacheDirPathStr);
            env->DeleteLocalRef(fileClass);
            env->DeleteLocalRef(cacheDir);
        }
    }

    env->DeleteLocalRef(contextClass);
    env->DeleteLocalRef(parentLoader);
    env->DeleteLocalRef(appContext);

    if (!dexClassLoader) return nullptr;

    jclass classLoaderClass = env->GetObjectClass(dexClassLoader);
    jmethodID loadClass = env->GetMethodID(classLoaderClass, "loadClass", "(Ljava/lang/String;)Ljava/lang/Class;");
    jstring className = env->NewStringUTF("com.rrq.overlay.NativeHelper");
    jclass helperClass = (jclass)env->CallObjectMethod(dexClassLoader, loadClass, className);

    env->DeleteLocalRef(className);
    env->DeleteLocalRef(classLoaderClass);
    env->DeleteLocalRef(dexClassLoader);

    if (!helperClass || env->ExceptionCheck()) { env->ExceptionClear(); return nullptr; }

    if (env->RegisterNatives(helperClass, g_NativeMethods, sizeof(g_NativeMethods) / sizeof(g_NativeMethods[0])) != JNI_OK) {
        env->DeleteLocalRef(helperClass);
        return nullptr;
    }

    jmethodID injectMethod = env->GetStaticMethodID(helperClass, "injectOverlay", "(Landroid/app/Activity;)V");
    if (!injectMethod) { env->DeleteLocalRef(helperClass); return nullptr; }
    
    env->CallStaticVoidMethod(helperClass, injectMethod, activity);
    if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(helperClass); return nullptr; }

    jmethodID getSurfaceViewMethod = env->GetStaticMethodID(helperClass, "getOverlaySurfaceView", "()Landroid/view/SurfaceView;");
    if (!getSurfaceViewMethod) { env->DeleteLocalRef(helperClass); return nullptr; }
    
    jobject surfaceView = env->CallStaticObjectMethod(helperClass, getSurfaceViewMethod);
    if (env->ExceptionCheck()) { env->ExceptionClear(); env->DeleteLocalRef(helperClass); return nullptr; }

    env->DeleteLocalRef(helperClass);
    return surfaceView;
}
