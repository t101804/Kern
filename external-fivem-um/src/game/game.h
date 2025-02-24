#pragma once
#include <cstdint>
#include "../../ext/ImGui 1.90/imgui.h"
#include <vector>
#include "Cped/cped.h"

// for debug because i'm not yet implementing the fivem features. i even not yet adding the offset lol XD
#define fivem_app L"FiveM_b2699_GTAProcess"


class FiveM {

public:
	void Setup();
	void RenderUpdate();
	void RenderEsp();
private:
	CPed local;
    std::vector<CPed> EntityList;
    uintptr_t GameWorld;
    uintptr_t ViewPort;

    // base address mem
    uintptr_t base_address;
    // Colors
    ImColor ESP_NPC = { 1.f, 1.f, 1.f, 1.f };
    ImColor ESP_PLAYER = { 1.f, 0.5f, 0.f, 1.f };
    ImColor ESP_GOD = { 1.f, 0.f, 0.f, 1.f };
    ImColor ESP_Skeleton = { 1.f, 1.f, 1.f, 1.f };
    ImColor ESP_Filled = { 0.f, 0.f, 0.f, 0.2f };
    ImColor FOV_User = { 1.f, 1.f, 1.f, 1.f };
    ImColor CrosshairColor = { 0.f, 1.f, 0.f, 1.f };

    void DrawLine(ImVec2 a, ImVec2 b, ImColor color, float width)
    {
        ImGui::GetBackgroundDrawList()->AddLine(a, b, color, width);
    }
    void RectFilled(float x0, float y0, float x1, float y1, ImColor color, float rounding, int rounding_corners_flags)
    {
        ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x0, y0), ImVec2(x1, y1), color, rounding, rounding_corners_flags);
    }
    void HealthBar(float x, float y, float w, float h, int value, int v_max)
    {
        RectFilled(x, y, x + w, y + h, ImColor(0.f, 0.f, 0.f, 0.725f), 0.f, 0);
        RectFilled(x, y, x + w, y + ((h / float(v_max)) * (float)value), ImColor(min(510 * (v_max - value) / 100, 255), min(510 * value / 100, 255), 25, 255), 0.0f, 0);
    }
    void ArmorBar(float x, float y, float w, float h, int value, int v_max)
    {
        RectFilled(x, y, x + w, y + h, ImColor(0.f, 0.f, 0.f, 0.725f), 0.f, 0);
        RectFilled(x, y, x + w, y + ((h / float(v_max)) * (float)value), ImColor(0.f, 1.f, 1.f, 1.f), 0.f, 0);
    }
    void Circle(ImVec2 pos, float fov_size, ImColor color)
    {
        ImGui::GetBackgroundDrawList()->AddCircle(pos, fov_size, color, 100, 0);
    }
    void String(ImVec2 pos, ImColor color, const char* text)
    {
        ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), ImGui::GetFontSize(), pos, color, text, text + strlen(text), 128, 0);
    }
    void StringEx(ImVec2 pos, ImColor color, float font_size, const char* text)
    {
        ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), font_size, ImVec2(pos.x + 1.f, pos.y + 1.f), ImColor(0.f, 0.f, 0.f, 1.f), text, text + strlen(text), 128, 0);
        ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), font_size, pos, color, text, text + strlen(text), 128, 0);
    }
};

//extern FiveM fivem;
