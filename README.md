##⚡ GLSurfaceView ImGui

Native Android ImGui Rendering Framework

GLSurfaceView ImGui is a native Android rendering framework built around Android GLSurfaceView, OpenGL ES, Dear ImGui, and the Android NDK.

The project provides a native-oriented rendering architecture for integrating Dear ImGui and OpenGL ES into Android applications while keeping the rendering layer lightweight, responsive, and integration-friendly.

##✨ Features

- ⚡ Native C/C++ rendering with Android NDK
- 🎨 Dear ImGui integration
- 🖥️ Android GLSurfaceView rendering
- 🔺 OpenGL ES rendering
- 👆 Touch and input handling
- 📱 Android 7.0+
- 🧩 ARMv7 and ARM64 support
- 🚀 Low-overhead rendering architecture
- 🛠️ Customizable rendering pipeline
- 🔧 Integration-friendly native architecture
- 📈 Performance and stability focused

---

##🧠 Architecture

Android Application
        │
        ▼
   GLSurfaceView
        │
        ▼
    OpenGL ES
        │
        ▼
    Dear ImGui
        │
        ▼
   Android NDK
        │
        ▼
   Native C/C++

The rendering pipeline is designed around Android's "GLSurfaceView" while moving the primary rendering implementation into native C/C++.

---

##🔥 Why GLSurfaceView ImGui?

Android's "GLSurfaceView" provides a convenient OpenGL rendering surface, while Dear ImGui provides an immediate-mode graphical user interface.

This project combines both with the Android NDK to provide a native rendering layer suitable for applications that require:

- Native OpenGL ES rendering
- Dear ImGui interfaces
- Custom rendering pipelines
- Native input processing
- Lightweight Android UI overlays
- C/C++ rendering infrastructure

---

##📱 Compatibility

Platform| Support
Android 7.0+| ✅
ARMv7| ✅
ARM64| ✅
GLSurfaceView| ✅
OpenGL ES| ✅
Android NDK| ✅
Dear ImGui| ✅

«Compatibility may vary depending on Android version, device hardware, GPU drivers, OpenGL ES implementation, and host application environment.»

---

##🎯 Design Goals

⚡ Performance

Native rendering with a focus on minimizing unnecessary overhead and maintaining responsive frame rendering.

🛡️ Stability

Designed with predictable rendering and input handling across different Android environments.

📱 Compatibility

Targeted at a broad range of Android devices and ARM architectures.

🔧 Flexibility

The rendering architecture is designed to be customized and integrated into different Android projects.

---

##🛠️ Technology Stack

- Android
- GLSurfaceView
- OpenGL ES
- Dear ImGui
- Android NDK
- C/C++
- JNI
- ARM / ARM64

---

##🚀 Integration

The native rendering library can be loaded from an Android application and initialized through the project's native interface.

Example:

const-string v0, "ImGui"
invoke-static {v0}, Ljava/lang/System;->loadLibrary(Ljava/lang/String;)V

The exact integration process depends on the host application's architecture and the way the native library is packaged.

---


##🔬 Development Focus

This project focuses on:

Native Android • GLSurfaceView • OpenGL ES • Dear ImGui • Android NDK • C/C++ • Rendering • Input • Optimization

The goal is to provide a clean foundation for experimentation and development involving native Android rendering technologies.

---

##🌐 RrQ Ecosystem

RrQ Mods

Projects, releases, updates, and resources.

Telegram: "@RrQ_Mods" (https://t.me/RrQ_Mods)

RrQ Owner

Project owner and developer.

Telegram: "@RrQ_Owner" (https://t.me/RrQ_Owner)

CoreEngineDevs

Development community focused on Native Android, rendering, and engine technologies.

Telegram: "CoreEngineDevs" (https://t.me/CoreEngineDevs)

---

##📄 License

MIT License
