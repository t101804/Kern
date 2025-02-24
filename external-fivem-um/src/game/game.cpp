#include "game.h"
#include <iostream>
#include <Windows.h>
#include <Tlhelp32.h>
#include <thread>
#include "../utils/driver.h"
#include "../utils/Log/log.h"
#include "Cped/cped.h"
#include "../Utils/Math/math.h"
#include "../utils/Config/config.h"

int bone = 0;
const int ReadCount = 256;

void FiveM::Setup() {
		bool driver = driver_manager::find_driver("\\\\.\\replican");
		if (!driver) {
			Logging::error_print("Failed to find driver");
			std::cin.get();
			return;
		}
		// attach to notepad
		const DWORD process_id = driver_manager::get_process_id(fivem_app);
		if (!process_id) {
			Logging::error_print("failed to get process id do you sure already opened fivem?");
			std::cin.get();
			return;
		}
		driver_manager::attach_to_process(process_id);
		base_address = driver_manager::get_module_base_address(process_id, fivem_app);
		if (!base_address) {
			Logging::error_print("failed to get base address");
			std::cin.get();
			return;
		}
		Logging::debug_print("Base address: " + std::to_string(base_address));
		Logging::debug_print("Process ID: " + std::to_string(process_id));
		
	/*	auto world = read_mem<uintptr_t>(base_address + offset::world);
		auto replay = read_mem<uintptr_t>(base_address + offset::replay);
		auto viewport = read_mem<uintptr_t>(base_address + offset::viewport);*/
	}

struct Entity {
    uint64_t address;
    uint64_t junk0;
};


struct EntityList_t {
    Entity entity[ReadCount];
};

void FiveM::RenderUpdate() {

	while (GlobalsConfig.Run) {
        std::vector<CPed> temp_pedlist;

        GameWorld = read_mem<uintptr_t>(base_address + sdk.world);
        local.address = read_mem<uintptr_t>(GameWorld + 0x8);
        ViewPort = read_mem<uintptr_t>(base_address + sdk.viewport);

        uintptr_t ReplayInterface = read_mem<uintptr_t>(base_address + sdk.replay);
        uintptr_t EntityListPtr = read_mem<uintptr_t>(ReplayInterface + 0x18);
        uintptr_t entitylist_addr = read_mem<uintptr_t>(EntityListPtr + 0x100);

        EntityList_t base = read_mem<EntityList_t>(entitylist_addr), * list = &base;

        for (int i = 0; i < ReadCount; i++)
        {
            if (list->entity[i].address != NULL && list->entity[i].address != local.address)
            {
                CPed p = CPed();

                p.address = list->entity[i].address;

                if (p.Update()) {
                    p.Index = i;
                    temp_pedlist.push_back(p);
                }
            }
        }

        this->EntityList = temp_pedlist;
        temp_pedlist.clear();

        Sleep(500);
	}
}

void FiveM::RenderEsp() {
	CPed* pLocal = &local;
    static CPed target = CPed();

    float MinFov = 9999.f;
    float MinDistance = 9999.f;
    Vector2 Center = Vector2(GlobalsConfig.GameRect.right / 2.f, GlobalsConfig.GameRect.bottom / 2.f);
    Matrix ViewMatrix = read_mem<Matrix>(ViewPort + 0x24C);

    if (!pLocal->Update())
        return;

    for (auto& entity : EntityList)
    {
        CPed* pEntity = &entity;

        // Checks
        if (!pEntity->Update())
            continue;

        float pDistance = GetDistance(pEntity->m_pVecLocation, pLocal->m_pVecLocation);

        if (pDistance > GlobalsConfig.ESP_MaxDistance)
            continue;
        else if (!GlobalsConfig.ESP_NPC && !pEntity->IsPlayer())
            continue;

        // Bone W2S
        Vector2 pBase{};
        if (!WorldToScreen(ViewMatrix, pEntity->m_pVecLocation, pBase))
            continue;

        std::vector<Vector2> bScreen;
        std::vector<Vector3> BoneList = pEntity->GetBoneList();

        // WorldToScreen
        for (int j = 0; j < BoneList.size(); j++)
        {
            Vector2 vOut{};

            if (Vec3_Empty(BoneList[j]))
                continue;
            else if (!WorldToScreen(ViewMatrix, BoneList[j], vOut))
                continue;

            bScreen.push_back(vOut);
        }

        if (BoneList.size() != bScreen.size())
            continue;

        // ESP Resource
        float HeadToNeck = bScreen[NECK].y - bScreen[HEAD].y;
        float pTop = bScreen[HEAD].y - (HeadToNeck * 2.5f);
        float pBottom = bScreen[LEFTFOOT].y > bScreen[RIGHTFOOT].y ? bScreen[LEFTFOOT].y : bScreen[RIGHTFOOT].y;
        float pHeight = pBottom - pTop;
        float pWidth = pHeight / 3.5f;
        float bScale = pWidth / 1.5f;
        ImColor color = pEntity->IsPlayer() ? ESP_PLAYER : ESP_NPC;

        // GodMode Checker
        if (pEntity->IsGod())
            color = ESP_GOD;

        // Line
        if (GlobalsConfig.ESP_Line)
            DrawLine(ImVec2(GlobalsConfig.GameRect.right / 2.f, GlobalsConfig.GameRect.bottom), ImVec2(pBase.x, pBottom), color, 1.f);

        // Box
        if (GlobalsConfig.ESP_Box)
        {
            if (GlobalsConfig.ESP_BoxFilled)
                ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(pBase.x - pWidth, pTop), ImVec2(pBase.x + pWidth, pBottom), ESP_Filled);

            switch (GlobalsConfig.ESP_BoxType)
            {
            case 0: // simple
                DrawLine(ImVec2(pBase.x - pWidth, pTop), ImVec2(pBase.x + pWidth, pTop), color, 1.f);
                DrawLine(ImVec2(pBase.x - pWidth, pTop), ImVec2(pBase.x - pWidth, pBottom), color, 1.f);
                DrawLine(ImVec2(pBase.x + pWidth, pTop), ImVec2(pBase.x + pWidth, pBottom), color, 1.f);
                DrawLine(ImVec2(pBase.x - pWidth, pBottom), ImVec2(pBase.x + pWidth, pBottom), color, 1.f);
                break;
            case 1: // cornerd
                DrawLine(ImVec2((pBase.x - pWidth), pTop), ImVec2((pBase.x - pWidth) + bScale, pTop), color, 1.f); // top
                DrawLine(ImVec2((pBase.x + pWidth), pTop), ImVec2((pBase.x + pWidth) - bScale, pTop), color, 1.f);
                DrawLine(ImVec2(pBase.x - pWidth, pTop), ImVec2(pBase.x - pWidth, pTop + bScale), color, 1.f); // left
                DrawLine(ImVec2(pBase.x - pWidth, pBottom), ImVec2(pBase.x - pWidth, pBottom - bScale), color, 1.f);
                DrawLine(ImVec2(pBase.x + pWidth, pTop), ImVec2(pBase.x + pWidth, pTop + bScale), color, 1.f); // right
                DrawLine(ImVec2(pBase.x + pWidth, pBottom), ImVec2(pBase.x + pWidth, pBottom - bScale), color, 1.f);
                DrawLine(ImVec2((pBase.x - pWidth), pBottom), ImVec2((pBase.x - pWidth) + bScale, pBottom), color, 1.f); // bottom
                DrawLine(ImVec2((pBase.x + pWidth), pBottom), ImVec2((pBase.x + pWidth) - bScale, pBottom), color, 1.f);
                break;
            default:
                break;
            }
        }

        // Skeleton
        if (GlobalsConfig.ESP_Skeleton)
        {
            // Head
            Circle(ImVec2(bScreen[HEAD].x, bScreen[HEAD].y), HeadToNeck, ESP_Skeleton);

            Vector3 bList[][2] = { { BoneList[NECK], BoneList[HIP] }, { BoneList[NECK], BoneList[LEFTHAND] }, { BoneList[NECK], BoneList[RIGHTHAND] },
                { BoneList[HIP], BoneList[LEFTANKLE] }, { BoneList[HIP], BoneList[RIGHTANKLE] } };

            // Body
            for (int j = 0; j < 5; j++)
            {
                Vector2 ScreenB1{}, ScreenB2{};
                if (Vec3_Empty(bList[j][0]) || Vec3_Empty(bList[j][1]))
                    break;

                if (!WorldToScreen(ViewMatrix, bList[j][0], ScreenB1) || !WorldToScreen(ViewMatrix, bList[j][1], ScreenB2))
                    break;

                DrawLine(ImVec2(ScreenB1.x, ScreenB1.y), ImVec2(ScreenB2.x, ScreenB2.y), ESP_Skeleton, 1.f);
            }
        }

        // Healthbar
        if (GlobalsConfig.ESP_HealthBar)
        {
            HealthBar((pBase.x - pWidth) - 5.f, pBottom, 2.f, -pHeight, pEntity->m_fHealth, pEntity->m_fMaxHealth);

            if (pEntity->m_fArmor > 0.f)
                ArmorBar((pBase.x + pWidth) + 3.f, pBottom, 2.f, -pHeight, pEntity->m_fArmor, 100);
        }

        // Distance
        if (GlobalsConfig.ESP_Distance)
        {
            std::string DistStr = std::to_string((int)pDistance) + "m";
            StringEx(ImVec2(pBase.x - ImGui::CalcTextSize(DistStr.c_str()).x / 2.f, pBottom), ImColor(1.f, 1.f, 1.f, 1.f), ImGui::GetFontSize(), DistStr.c_str());
        }

        // Name
        if (GlobalsConfig.ESP_Name)
        {
            /* - GTA Online
            std::string pName = m.ReadString(pEntity->PlayerInfo + 0xFC);
            StringEx(ImVec2(pBase.x - ImGui::CalcTextSize(pName.c_str()).x / 2.f, pTop - 15.f), ImColor(1.f, 1.f, 1.f, 1.f), ImGui::GetFontSize(), pName.c_str());
            */
        }

        // AimBot
       /* if (GlobalsConfig.AimBot)
        {
            if (IsKeyDown(g.AimKey0) || IsKeyDown(g.AimKey1))
            {
                if (g.Aim_MaxDistance < pDistance)
                    continue;
                else if (!g.Aim_NPC && !pEntity->IsPlayer())
                    continue;

                for (int j = 0; j < 9; j++)
                {
                    Vector2 fov_check{};
                    if (!WorldToScreen(ViewMatrix, BoneList[j], fov_check))
                        continue;

                    float FOV = abs((Center - fov_check).Length());

                    if (FOV < g.Aim_Fov)
                    {
                        switch (g.Aim_Type)
                        {
                        case 0:
                            if (FOV < MinFov)
                            {
                                target = entity;
                                MinFov = FOV;

                                break;
                            }
                            break;
                        case 1:
                            if (pDistance < MinDistance)
                            {
                                target = entity;
                                MinDistance = pDistance;

                                break;
                            }
                            break;
                        default:
                            break;
                        }
                    }
                }
            }*/
        }
}