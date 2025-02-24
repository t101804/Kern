#pragma once
#include <d3d11.h>
#include <dxgi.h>

#include "../../ext/ImGui 1.90/imgui.h"
#include "../../ext/ImGui 1.90/imgui_impl_dx11.h"
#include "../../ext/ImGui 1.90/imgui_impl_win32.h"

class Overlay {
private:
	// creation of device, window, and ImGui.
	bool CreateDevice();
	void CreateOverlay(const wchar_t* window_name);
	bool CreateImGui();

	// destruction of device, window and ImGui.
	void DestroyDevice();
	void DestroyOverlay();
	void DestroyImGui();

	// returns 60.f if it fails.
	float GetRefreshRate();

	
// for utils imgui
private:
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
public:
	void SetupOverlay(const wchar_t* window_name) {
		CreateOverlay(window_name);
		CreateDevice();
		CreateImGui();
	}

	// deconstructor called when program is exiting. THIS IS CALLED AUTOMATICALLY, NO NEED TO CALL IT YOURSELF!
	~Overlay() {
		DestroyDevice();
		DestroyOverlay();
		DestroyImGui();
	}

	// if we should render menu on top of the overlay.
	bool RenderMenu;

	bool ESP;
	// if we should run the overlay at all
	bool shouldRun = true;

	// for use inside of your main loop to render.
	void StartRender();
	void EndRender();

	// i'm using this for the menu specifically, but you could render ESP and shit like that in here if you wanted to.
	// as long as you're calling ImGui::GetBackgroundDrawList() between Start and End render you're fine to do what you want :)

	// NOTE: if the user presses the x in the titlebar, the styles will not be set correctly. This can be fixed by checking the renderMenu bool every frame.
	// (which is really bad for performance)
	void RenderMenuGui();

	// small helper functions for the SetForeground function
	bool IsWindowInForeground(HWND window) { return GetForegroundWindow() == window; }
	bool BringToForeground(HWND window) { return SetForegroundWindow(window); }

	// sets the window to the foreground
	void SetForeground(HWND window);
private:
	// winapi window requirements
	HWND overlay;
	WNDCLASSEX wc;

	ID3D11Device* device;
	ID3D11DeviceContext* device_context;
	IDXGISwapChain* swap_chain;
	ID3D11RenderTargetView* render_targetview;
};