LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := ImGui

LOCAL_C_INCLUDES := $(LOCAL_PATH)/CoreEngine $(LOCAL_PATH)/ImGui

LOCAL_SRC_FILES := main.cpp \
                   ImGui/imgui.cpp \
                   ImGui/imgui_draw.cpp \
                   ImGui/imgui_tables.cpp \
                   ImGui/imgui_widgets.cpp \
                   ImGui/imgui_impl_opengl3.cpp

LOCAL_LDLIBS := -lEGL -lGLESv3 -landroid -llog -ldl

LOCAL_CPPFLAGS := -std=c++17 -fvisibility=hidden -fexceptions -frtti -Wno-unknown-warning-option
LOCAL_CFLAGS := -O3 -Wall -Wno-unknown-warning-option

include $(BUILD_SHARED_LIBRARY)

