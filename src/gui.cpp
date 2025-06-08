#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif

#include "imgui.h"

#include "imgui-knobs.h"

#include "gui.h"
#include "audio.h"
#include "filemanager.h"
#include "widgets.h"

#include <iostream>

void GUI::mainInterface(AudioEngine& audioEngine) {
    ImGui::Begin("interface", NULL, ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoTitleBar);

    ImVec2 scopeSize = ImVec2(ImGui::GetContentRegionAvail().x,100);

    ImGui::SeparatorText(FileManager::currentFileName.c_str());

    ImGui::PushID(0);
    ImVec2 plotPos = ImGui::GetCursorScreenPos();
    ImGui::SetNextItemAllowOverlap();
    // Render audio file waveform
    Widgets::PlotLines(
        "##audio", 
        audioEngine.audioData.samples.data(), 
        audioEngine.audioData.samples.size(), 
        0, nullptr, -1.0f, 1.0f, scopeSize
    );
    // Render playhead
    ImGui::SetCursorScreenPos(plotPos);
    Widgets::Playhead(1.0f * audioEngine.index / audioEngine.audioData.size, scopeSize);
    ImGui::PopID(); //0

    // Button or space bar to play/pause 
    if(ImGui::Button("Play/Pause") || ImGui::IsKeyPressed(ImGuiKey_Space, false)) {
        audioEngine.play = !audioEngine.play;
    }
    ImGui::SameLine();
    if(ImGui::Button("Stop")) {
        audioEngine.play = false;
        audioEngine.index = 0;
    }
    ImGui::SameLine();
    Widgets::Checkbox("Loop", &audioEngine.loop);

    audioEngine.panner.drawInterface();

    ImGuiKnobs::Knob("Volume", &audioEngine.masterVolume, 0.0f, 2.0f);

    ImGui::End();
}

void GUI::loadFile() {
    ImGui::Begin("load file window", NULL, ImGuiWindowFlags_NoResize | 
        ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove | 
        ImGuiWindowFlags_NoTitleBar);
    const char* text;
        if (FileManager::loading)
            text = "Loading file...";
        else
            text = "Please drag and drop an audio file\nin one of the following formats:\n.wav, .flac or .mp3";
        ImVec2 textSize = ImGui::CalcTextSize(text, ImGui::FindRenderedTextEnd(text));
        ImGui::SetCursorPos(ImVec2(
            ImGui::GetWindowWidth() / 2 - textSize.x / 2,
            ImGui::GetWindowHeight() / 2 - textSize.y / 2
        ));
        ImGui::Text("%s", text);
    ImGui::End();
}