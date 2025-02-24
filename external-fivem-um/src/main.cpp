#include <iostream>
#include <Windows.h>
#include <Tlhelp32.h>
#include "Game/game.h"
#include "utils/Config/config.h"
#include <thread>
#include "Window/Overlay/overlay.h"

FiveM* fivem = new FiveM;
Overlay* ovrlay = new Overlay;


int main() {	
	std::thread([&]() { fivem->Setup(); }).detach();
	if (!ovrlay->InitOverlay(L"grcWindow", WINDOW_CLASS))
		return 2;
	ovrlay->OverlayLoop(fivem);
	ovrlay->DestroyOverlay();
	delete ovrlay;
	delete fivem;
	return 0;
}