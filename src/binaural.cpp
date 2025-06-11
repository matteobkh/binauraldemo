#include "binaural.h"

#include "imgui.h"
#include "imgui_internal.h"

#include <cmath>

// positiveX=true returns 0 for x<0 and scaled values for x>0, false does the opposite
float cosScale(float x, bool positiveX) {
    x /= 180.0f;
    float y = (cosf(2*M_PI*(x + 0.5f)) + 1.0f) / 2.0f;
    if (positiveX) {
        return x > 0 ? y : 0;
    } else {
        return x < 0 ? y : 0;
    }
}

// Use in main audio loop
void BinauralPanner::process(float in, float& l, float& r) {
    // clip distance
    if (dist < 0.333f) {
        dist = 0.333f;
    } else if (dist > 1.0f) {
        dist = 1.0f;
    }
    // scale distance and calculate gain reduction
    // subtract -0.333 cause thats minimum dist
    float dist_scaled = -3.0f * (dist - 0.333f); //if k=-3 -> -12dB max gr, -2 -> -6dB
    float gr = pow(2.0f, dist_scaled);
    delay.write(in);
    // LEF CHANNEL: if angle -90 -> delay min (0 samples)
    l = filterL.processMono( // apply head filtering
        delay.read(static_cast<int>(cosScale(angle,true) * MAX_DELAY)) // apply delay
        * (MIN_AMP + DELTA_AMP * cosScale(angle,false)) // apply amplitude
    ) * gr;
    // RIGHT CHANNEL: if angle -90 -> delay max (44 samples, approx 1ms)
    r = filterR.processMono( // apply head filtering
       r = delay.read(static_cast<int>(cosScale(angle,false) * MAX_DELAY)) // apply delay
       * (MIN_AMP + DELTA_AMP * cosScale(angle, true)) // apply amplitude
    ) * gr;
}

// Use this function to update angle of panner
void BinauralPanner::update(float theta) {
    angle = theta;
    // LEFT: if angle -90 -> max cutoff (20k) 
    filterL.updateCutoff(MIN_CUTOFF + cosScale(angle, true) * DELTA_CUTOFF);
    // RIGHT: if angle -90 -> min cutoff (8k)
    filterR.updateCutoff(MIN_CUTOFF + (1.0f - cosScale(angle, false)) * DELTA_CUTOFF);
}

void BinauralPanner::drawInterface() {
    // -- spatializer interface --
    // get window padding
    float padding = ImGui::GetStyle().WindowPadding.x;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 p = ImGui::GetCursorScreenPos();
    float size = ImGui::GetContentRegionAvail().x;
    ImVec2 center = ImVec2(p.x + size * 0.5f, p.y + size * 0.5f);
    float radius = size * 0.45f;

    // Style colors
    ImU32 bg_col = ImGui::GetColorU32(ImGuiCol_FrameBg);
    ImU32 line_col = ImGui::GetColorU32(ImGuiCol_Text);

    // Background square
    draw_list->AddRectFilled(p, ImVec2(p.x + size, p.y + size * 0.55f), bg_col);

    // Concentric semicircles
    for (int i = 1; i <= 3; ++i) {
        float r = radius * i / 3.0f;
        draw_list->PathArcTo(center, r, IM_PI, 2 * IM_PI);
        draw_list->PathStroke(line_col, false, 1.0f);
    }

    // Radial lines
    for (int i = 0; i < 5; ++i) {
        float angle = IM_PI + (i * (IM_PI / 4.0f));
        ImVec2 end = ImVec2(center.x + cosf(angle) * radius, center.y + sinf(angle) * radius);
        draw_list->AddLine(center, end, line_col, 1.0f);
    }

    ImGui::Text("Angle: %.2f°", angle);
    ImGui::Text("Distance: %.2f", dist);

    int sz = 10;

    // Sound source placement
    float x = (sin(angle / 180.0f * M_PI) * (dist/2.22222f) + 0.5f) * size;
    float y = (cos(angle / 180.0f * M_PI) * (dist/2.22222f)) * size;

    ImGui::SetCursorPos(ImVec2(p.x + x - sz/2, p.y + size/2 - y - sz/2));

    ImGui::Button("##pos", ImVec2(sz,sz));
    if (ImGui::IsItemActive()) {
        ImVec2 mousepos = ImGui::GetIO().MousePos;
        float newX = 1.0f * (mousepos.x-size/2-padding) / floor(size);
        float newY = 1.0f * (-mousepos.y+p.y+size/2) / floor(size);
        if (newY < 0.0f) { newY = 0; } // clip Y
        angle = atan(newX / newY) * 180.0f / M_PI;
        dist = sqrt(newX*newX + newY*newY) * 2.22222f; 
    }

    //double click to reset default, NOT WORKING
    /* if (ImGui::IsItemActive() && ImGui::IsMouseDoubleClicked(0)) { 
        angle = 0.0f;
        dist = 0.333f;
    } */

    // reset cursor position
    ImGui::SetCursorPos(ImVec2(p.x, p.y + size*0.55f+padding));
}