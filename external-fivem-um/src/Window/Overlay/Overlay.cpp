#include "overlay.h"
#include <TlHelp32.h>
#include "../../Utils/Config/config.h"
#include "../../../ext/ImGui 1.90/imgui.h"
#include "../../../ext/ImGui 1.90/imgui_impl_win32.h"
#include "../../../ext/ImGui 1.90/imgui_impl_dx11.h"
#include "../../Utils/Log/log.h"
#include <d3d11.h>
//#include "../../Utils/Util.cpp"

void LoadStyle();
LONG MenuStyle = WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST;
LONG ESPStyle = WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST;

ID3D11Device* g_pd3dDevice = NULL;
ID3D11DeviceContext* g_pd3dDeviceContext = NULL;
IDXGISwapChain* g_pSwapChain = NULL;
ID3D11RenderTargetView* g_mainRenderTargetView = NULL;

void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
bool CreateDeviceD3D(HWND hWnd);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);


bool Overlay::InitOverlay(const std::wstring targetName, int mode)
{
    if (mode == WINDOW_TITLE || mode == WINDOW_CLASS) {
        TargetHwnd = WINDOW_TITLE ? FindWindowW(nullptr, targetName.c_str()) : FindWindowW(targetName.c_str(), nullptr);

        if (!TargetHwnd)
            return false;
    }
    else if (mode == PROCESS) {
        TargetHwnd = GetTargetWindow(targetName);

        if (!TargetHwnd)
            return false;
    }
    else {
        return false;
    }

    return CreateOverlay();
}


void Overlay::DestroyOverlay()
{
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    DestroyWindow(Hwnd);
    UnregisterClassA(wc.lpszClassName, wc.hInstance);
}

bool Overlay::CreateOverlay()
{
    // Get ClientSize
    GetClientRect(GlobalsConfig.GameHwnd, &GlobalsConfig.GameRect);
    ClientToScreen(GlobalsConfig.GameHwnd, &GlobalsConfig.GamePoint);

    // Create Overlay
    wc = { sizeof(WNDCLASSEXA), 0, WndProc, 0, 0, NULL, NULL, NULL, NULL, Title, Class, NULL };
    RegisterClassExA(&wc);
    Hwnd = CreateWindowExA(WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW | WS_EX_TOPMOST, wc.lpszClassName, wc.lpszMenuName, WS_POPUP | WS_VISIBLE, GlobalsConfig.GamePoint.x, GlobalsConfig.GamePoint.y, GlobalsConfig.GameRect.right, GlobalsConfig.GameRect.bottom, NULL, NULL, wc.hInstance, NULL);
    SetLayeredWindowAttributes(Hwnd, RGB(0, 0, 0), 255, LWA_ALPHA);
    MARGINS margin = { -1 };
    DwmExtendFrameIntoClientArea(Hwnd, &margin);

    if (!CreateDeviceD3D(Hwnd))
    {
        CleanupDeviceD3D();
        UnregisterClassA(wc.lpszClassName, wc.hInstance);

        return false;
    }

    ShowWindow(Hwnd, SW_SHOWDEFAULT);
    UpdateWindow(Hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = nullptr;
    io.LogFilename = nullptr;

    LoadStyle();
    ImGui_ImplWin32_Init(Hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    return true;
}

HWND Overlay::GetTargetWindow(const std::wstring processName)
{
    DWORD PID = NULL;
    PROCESSENTRY32 entry{};
    entry.dwSize = sizeof(PROCESSENTRY32);

    const auto snapShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    while (Process32Next(snapShot, &entry))
    {
        if (!processName.compare(entry.szExeFile))
        {
            PID = entry.th32ProcessID;
            break;
        }
    }

    CloseHandle(snapShot);

    HWND hwnd = GetTopWindow(NULL);
    do {
        if (GetWindowLong(hwnd, 8) != 0 || !IsWindowVisible(hwnd))
            continue;
        DWORD ProcessID;
        GetWindowThreadProcessId(hwnd, &ProcessID);
        if (PID == ProcessID)
            return hwnd;
    } while ((hwnd = GetNextWindow(hwnd, GW_HWNDNEXT)) != NULL);

    return NULL;
}


bool IsKeyDown(int VK)
{
    return (GetAsyncKeyState(VK) & 0x8000) != 0;
}

void Overlay::RenderMenuGui()
{
    // Setup
    ImVec4* colors = ImGui::GetStyle().Colors;
    ImGuiStyle& style = ImGui::GetStyle();
    static float DefaultSpacing = style.ItemSpacing.y;
    static int Index = 0;
    static int BindingID = 0;

    ImGui::SetNextWindowBgAlpha(0.975f);
    ImGui::SetNextWindowSize(ImVec2(725.f, 450.f));

    ImGui::Begin("baru pertama kali bang bikin kernel", &GlobalsConfig.ShowMenu, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
    ImGui::BeginChild("##LeftChild", ImVec2(ImGui::GetContentRegionAvail().x / 2.f - 16.f, ImGui::GetContentRegionAvail().y), false);
    ImGui::Spacing();
    ImGui::Text("Visual");
    ImGui::Separator();
    ImGui::Spacing();
    ImGui::Checkbox("ESP", &GlobalsConfig.ESP);
    ImGui::NewLine();
    ImGui::Spacing();

    ImGui::Text("ESP Options");
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::Checkbox("Box", &GlobalsConfig.ESP_Box);
    ImGui::Checkbox("BoxFilled", &GlobalsConfig.ESP_BoxFilled);
    ImGui::Checkbox("Line", &GlobalsConfig.ESP_Line);
    ImGui::Checkbox("Name", &GlobalsConfig.ESP_Name);
    ImGui::Checkbox("Skeleton", &GlobalsConfig.ESP_Skeleton);
    ImGui::Checkbox("Distance", &GlobalsConfig.ESP_Distance);
    ImGui::Checkbox("HealthBar", &GlobalsConfig.ESP_HealthBar);

    ImGui::EndChild();
    //ImGui::BeginChild("##RightChild", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y), false);

    // Calculate available width and text width
    //float availWidth = ImGui::GetContentRegionAvail().x;
    //float textWidth = ImGui::CalcTextSize("Setting").x;

    //// Set the cursor position so that the taaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa
    //ImGui::SetCursorPosX(availWidth - textWidth);
    //ImGui::Text("Setting");
    //ImGui::Separator();
    //ImGui::Spacing();
    //ImGui::Checkbox("Bx", &GlobalsConfig.ESP_Box);
    //ImGui::EndChild();

    ImGui::End();
}

void Overlay::OverlayLoop(FiveM* fivem)
{
    while (GlobalsConfig.Run)
    {
        MSG msg;
        while (PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE)) {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        OverlayManager("grcWindow");

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        if (GlobalsConfig.ESP) {
            fivem->RenderEsp();
        }
        if (GlobalsConfig.ShowMenu) {
            Overlay:RenderMenuGui();
        }
        //OverlayUserFunction();

        ImGui::Render();
        const float clear_color_with_alpha[4] = { 0.f, 0.f, 0.f, 0.f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

        g_pSwapChain->Present(1, 0);
    }
}
void Overlay::OverlayManager(const char* targetName)
{
    // Window Check
    HWND CheckHwnd = FindWindowA(targetName, nullptr);
    if (!CheckHwnd) {
        GlobalsConfig.Run = false;
        Logging::debug_print("target name not found: " );
        return;
    }

    // StreamProof
    DWORD Flag = NULL;
    GetWindowDisplayAffinity(Hwnd, &Flag);
    if (GlobalsConfig.StreamProof && Flag == WDA_NONE)
        SetWindowDisplayAffinity(Hwnd, WDA_EXCLUDEFROMCAPTURE);
    else if (!GlobalsConfig.StreamProof && Flag == WDA_EXCLUDEFROMCAPTURE)
        SetWindowDisplayAffinity(Hwnd, WDA_NONE);

    // Window Style Changer
    HWND ForegroundWindow = GetForegroundWindow();
    LONG TmpLong = GetWindowLong(Hwnd, GWL_EXSTYLE);

    if (GlobalsConfig.ShowMenu && MenuStyle != TmpLong)
        SetWindowLong(Hwnd, GWL_EXSTYLE, WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST);
    else if (!GlobalsConfig.ShowMenu && ESPStyle != TmpLong)
        SetWindowLong(Hwnd, GWL_EXSTYLE, WS_EX_TRANSPARENT | WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_TOPMOST);

    // ShowMenu
    static bool menu_key = false;
    if (IsKeyDown(GlobalsConfig.MenuKey) && !menu_key)
    {
        GlobalsConfig.ShowMenu = !GlobalsConfig.ShowMenu;

        if (GlobalsConfig.ShowMenu && ForegroundWindow != Hwnd)
            SetForegroundWindow(Hwnd);
        else if (!GlobalsConfig.ShowMenu && ForegroundWindow != CheckHwnd)
            SetForegroundWindow(CheckHwnd);

        menu_key = true;
    }
    else if (!IsKeyDown(GlobalsConfig.MenuKey) && menu_key)
    {
        menu_key = false;
    }

    // Window Resizer
    RECT TmpRect{};
    POINT TmpPoint{};
    GetClientRect(CheckHwnd, &TmpRect);
    ClientToScreen(CheckHwnd, &TmpPoint);

    // Resizer
    if (TmpRect.left != GlobalsConfig.GameRect.left || TmpRect.bottom != GlobalsConfig.GameRect.bottom || TmpRect.top != GlobalsConfig.GameRect.top || TmpRect.right != GlobalsConfig.GameRect.right || TmpPoint.x != GlobalsConfig.GamePoint.x || TmpPoint.y != GlobalsConfig.GamePoint.y)
    {
        GlobalsConfig.GameRect = TmpRect;
        GlobalsConfig.GamePoint = TmpPoint;
        SetWindowPos(Hwnd, nullptr, TmpPoint.x, TmpPoint.y, GlobalsConfig.GameRect.right, GlobalsConfig.GameRect.bottom, SWP_NOREDRAW);
    }
}


//void Overlay::OverlayUserFunction()
//{
//    cheat->MiscAll();
//
//    cheat->RenderInfo();
//
//    if (g.ESP)
//        cheat->RenderESP();
//
//    if (g.ShowMenu)
//        cheat->RenderMenu();
//}

void LoadStyle()
{
    ImGuiStyle& style = ImGui::GetStyle();

    // Border
    style.WindowBorderSize = 0.f;
    style.ChildBorderSize = 1.f;
    style.PopupBorderSize = 1.f;
    style.FrameBorderSize = 0.f;
    style.TabBorderSize = 1.f;
    style.TabBarBorderSize = 0.f;

    // Rounding
    style.WindowRounding = 0.f;
    style.ChildRounding = 0.f;
    style.FrameRounding = 0.f;
    style.PopupRounding = 0.f;
    style.TabRounding = 0.f;

    // Misc
    style.ScrollbarSize = 3.f;
    style.GrabMinSize = 5.f;
    style.SeparatorTextBorderSize = 1.f;
    style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
    style.DisplayWindowPadding = ImVec2(0.f, 0.f);       // Window position are clamped to be visible within the display area or monitors by at least this amount. Only applies to regular windows.
    style.DisplaySafeAreaPadding = ImVec2(50.f, 50.f);     // If you cannot see the edges of your screen (e.g. on a TV) increase the safe area padding. Apply to popups/tooltips as well regular windows. NB: Prefer configuring your TV sets correctly!

    ImVec4* colors = ImGui::GetStyle().Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_Border] = ImVec4(0.27f, 0.27f, 0.27f, 0.50f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.21f, 0.21f, 0.21f, 1.00f);
    colors[ImGuiCol_CheckMark] = ImVec4(0.45f, 0.45f, 0.81f, 1.00f);
    colors[ImGuiCol_SliderGrab] = ImVec4(0.39f, 0.39f, 0.78f, 1.00f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.35f, 0.35f, 0.78f, 1.00f);
    colors[ImGuiCol_Button] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
    colors[ImGuiCol_Header] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
    colors[ImGuiCol_HeaderHovered] = ImVec4(0.24f, 0.24f, 0.24f, 1.00f);
    colors[ImGuiCol_HeaderActive] = ImVec4(0.33f, 0.33f, 0.33f, 1.00f);
    colors[ImGuiCol_Separator] = ImVec4(0.27f, 0.27f, 0.27f, 0.50f);
    colors[ImGuiCol_Tab] = ImVec4(0.18f, 0.18f, 0.18f, 1.00f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.13f, 0.13f, 0.13f, 0.97f);
    colors[ImGuiCol_TabActive] = ImVec4(0.09f, 0.09f, 0.09f, 1.00f);
    colors[ImGuiCol_TitleBg] = ImVec4(0.45f, 0.45f, 0.81f, 1.00f);
    colors[ImGuiCol_TitleBgActive] = ImVec4(0.45f, 0.45f, 0.81f, 1.00f);
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}


bool CreateDeviceD3D(HWND hWnd)
{
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}



extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
        {
            CleanupRenderTarget();
            g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
            CreateRenderTarget();
        }
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU)
            return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProcA(hWnd, msg, wParam, lParam);
}
