#pragma once
#include <Windows.h>

struct Cfg
{
    // System
    bool Run = true;
    bool ShowMenu = false;

    // GameData
    HWND GameHwnd;
    RECT GameRect{};
    POINT GamePoint{};

    // AimBot
    bool AimBot = true;
    bool Aim_NPC = false;
    bool Aim_Prediction = true;
    bool Aim_DrawFov = true;

    float Aim_Fov = 150.f;
    float Aim_Smooth = 1.f;
    float Aim_MaxDistance = 150.f;
    float Aim_Predict = 2000.f;

    int Aim_Bone = 0; // Bone
    int Aim_Type = 0; // FOV/Dist
    int AimKeyType = 1; // and/or

    // Visual
    bool ESP = true;
    bool ESP_NPC = false;
    bool ESP_Box = false;
    bool ESP_BoxFilled = false;
    bool ESP_Line = false;
    bool ESP_Name = false;
    bool ESP_Skeleton = false;
    bool ESP_Distance = false;
    bool ESP_HealthBar = false;
    int ESP_BoxType = 1;
    float ESP_MaxDistance = 500.f;

    // Local
    bool GodMode = false;
    bool HealMe = false;
    bool NoRecoil = false;
    bool NoSpread = false;

    // System
    bool StreamProof = false;
    bool Crosshair = false;
    int CrosshairType = 0;
    int CrosshairSize = 3;

    // Key
    int MenuKey = VK_INSERT;
    int AimKey0 = VK_RBUTTON;
    int AimKey1 = VK_LBUTTON;
};

extern Cfg GlobalsConfig;