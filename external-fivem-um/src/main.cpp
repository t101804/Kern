#include <iostream>
#include <Windows.h>
#include <Tlhelp32.h>
#include "Window/window.hpp"
#include "Game/game.h"
#include "utils/Config/config.h"
#include <thread>

FiveM* fivem = new FiveM;

// just a demo to interact with repdriver
// in this usermode program we will open a handle to repdriver to read and write memory from another process
// in this section i will read memory from a fivem proccess

int main() {
	Overlay overlay;
	overlay.SetupOverlay(L"fivem external kernel");
	
	std::thread([&]() { fivem->Setup(); }).detach();
	while (overlay.shouldRun) {
		//std::this_thread::sleep_for(std::chrono::milliseconds(1));	
		overlay.StartRender();
		if (overlay.RenderMenu) {
			overlay.Render();
		}
		if (&GlobalsConfig.ESP) {
			fivem->RenderEsp();
		}
		overlay.EndRender();
	}

	return 0;
}