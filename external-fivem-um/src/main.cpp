#include <iostream>
#include <Windows.h>
#include <Tlhelp32.h>
#include "window/window.hpp"
#include "game/game.h"
#include <thread>

// just a demo to interact with repdriver
// in this usermode program we will open a handle to repdriver to read and write memory from another process
// in this section i will read memory from a fivem proccess

int main() {
	//ShowWindow(GetConsoleWindow(), SW_HIDE);
	Overlay overlay;
	overlay.SetupOverlay(L"fivem external");

	FiveM::Setup();
	while (overlay.shouldRun) {
		std::this_thread::sleep_for(std::chrono::milliseconds(1));	
		overlay.StartRender();
		if (overlay.RenderMenu) {
			overlay.Render();
		}
		overlay.EndRender();
	}

	return 0;
}