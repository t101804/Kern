#include "game.h"
#include <iostream>
#include <Windows.h>
#include <Tlhelp32.h>
#include <thread>
#include "../utils/driver.h"
#include "../utils/log.h"

void FiveM::Setup() {
		bool driver = driver_manager::find_driver("\\\\.\\replicant");
		if (!driver) {
			Logging::error_print("Failed to find driver");
			std::cin.get();
			return;
		}
		// attach to notepad
		const DWORD process_id = driver_manager::get_process_id(fivem_app);
		if (!process_id) {
			Logging::error_print("failed to get process id");
			std::cin.get();
			return;
		}
		driver_manager::attach_to_process(process_id);
		const std::uintptr_t base_address = driver_manager::get_module_base_address(process_id, fivem_app);
		if (!base_address) {
			Logging::error_print("failed to get base address");
			std::cin.get();
			return;
		}
		Logging::debug_print("Base address: " + std::to_string(base_address));
		Logging::debug_print("Process ID: " + std::to_string(process_id));

	}
