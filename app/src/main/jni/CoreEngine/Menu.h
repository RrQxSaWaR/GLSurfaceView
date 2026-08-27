#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include "Globals.h"

inline void RenderImGuiContent() {
    ImGuiIO& io = ImGui::GetIO();
    float screenW = io.DisplaySize.x;
    float screenH = io.DisplaySize.y;

    // 1. Bottom Centered Name Banner
    const char* watermarkText = "TeleGram: @RrQ_Owner";
    ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f, screenH - 40.0f), ImGuiCond_Always, ImVec2(0.5f, 1.0f));
    ImGui::SetNextWindowBgAlpha(0.0f);
    
    ImGui::Begin("BottomName", nullptr,
                 ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
                 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar |
                 ImGuiWindowFlags_NoInputs | ImGuiWindowFlags_NoBackground | 
                 ImGuiWindowFlags_AlwaysAutoResize);
                 
    ImGui::SetWindowFontScale(1.3f); 
    ImGui::TextColored(ImVec4(0.0f, 0.85f, 1.0f, 1.0f), "%s", watermarkText);
    ImGui::SetWindowFontScale(1.0f); 
    ImGui::End();

    // 2. Main Control Menu
    if (g_ShowMenu) {
        ImGui::SetNextWindowSize(ImVec2(850, 550), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowPos(ImVec2(screenW * 0.5f, screenH * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowBgAlpha(g_MenuAlpha);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 10.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 6.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(10.0f, 15.0f)); 

        ImGui::Begin("Native Draw RrQ_Owner", &g_ShowMenu);

        ImGui::TextColored(ImVec4(1.0f, 0.84f, 0.0f, 1.0f), "GLSurfaceView Engine");
        ImGui::Separator();
        ImGui::Spacing();

        if (ImGui::BeginTabBar("MainTabs")) {
            if (ImGui::BeginTabItem("Display & Style")) {
                ImGui::Spacing();
                ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Customize Menu Appearance");
                ImGui::Spacing();
                
                ImGui::SliderFloat("Menu Alpha", &g_MenuAlpha, 0.2f, 1.0f, "%.2f");
                ImGui::SliderFloat("Custom Slider", &g_CustomSlider, 0.0f, 1.0f, "%.2f");
                
                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();
                
                if (ImGui::Button("Reset Defaults", ImVec2(220, 45))) {
                    g_MenuAlpha = 0.90f;
                    g_CustomSlider = 0.5f;
                }
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }

        ImGui::End();
        ImGui::PopStyleVar(3); 
    }
}
