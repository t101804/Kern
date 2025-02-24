#include "driver.h"
#include <Tlhelp32.h>

uintptr_t processId;
uintptr_t baseId;

HANDLE m_driver_handle;

bool driver_manager::find_driver(const char* driver_name) {
	m_driver_handle = CreateFileA(driver_name, GENERIC_READ, 0, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
	//driver_handle = CreateFileW((L"\\\\.\\pymodule"), GENERIC_READ | GENERIC_WRITE, FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
	if (!m_driver_handle || (m_driver_handle == INVALID_HANDLE_VALUE))
		return false;

	return true;
}

void driver_manager::read_virtual_memory(PVOID address, PVOID buffer, DWORD size) {
	info_t io_info;
	io_info.target_address = address;
	io_info.buffer_address = buffer;
	io_info.size = size;
	DeviceIoControl(m_driver_handle, codes::read, &io_info, sizeof(io_info), &io_info, sizeof(io_info), nullptr, nullptr);
}

void driver_manager::write_virtual_memory(PVOID address, PVOID buffer, DWORD size) {
	info_t io_info;
	io_info.target_address = address;
	io_info.buffer_address = buffer;
	io_info.size = size;
	DeviceIoControl(m_driver_handle, codes::write, &io_info, sizeof(io_info), &io_info, sizeof(io_info), nullptr, nullptr);
}

void driver_manager::attach_to_process(DWORD process_id) {
	info_t io_info;

	io_info.target_pid = reinterpret_cast<HANDLE>(process_id);

	DeviceIoControl(m_driver_handle, codes::init, &io_info, sizeof(io_info), &io_info, sizeof(io_info), nullptr, nullptr);
}


DWORD driver_manager::get_process_id(const wchar_t* process_name) {
	DWORD process_id = 0;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (snapshot == INVALID_HANDLE_VALUE) {
		return process_id;
	}
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(entry);
	if (Process32First(snapshot, &entry)) {
		if (!_wcsicmp(entry.szExeFile, process_name)) {
			process_id = entry.th32ProcessID;
		}
		else {
			while (Process32Next(snapshot, &entry)) {
				if (!_wcsicmp(entry.szExeFile, process_name)) {
					process_id = entry.th32ProcessID;
					break;
				}
			}
		}

	}
	CloseHandle(snapshot);
	return process_id;
}
//uintptr_t driver_manager::get_base_address(DWORD process_id)  {
//	char pModule[128]{};
//
//	GetModuleBaseNameA("FiveM_b2699_GTAProcess", nullptr, pModule, sizeof(pModule)); // FiveM_b2699_GTAProcess
//	return get_module_base_address(process_id,pModule);
//}

std::uintptr_t driver_manager::get_module_base_address(DWORD process_id, const wchar_t* module_name) {
	std::uintptr_t base_address = 0;
	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
	if (snapshot == INVALID_HANDLE_VALUE) {
		return base_address;
	}
	MODULEENTRY32 entry;
	entry.dwSize = sizeof(entry);
	if (Module32First(snapshot, &entry)) {
		if (!_wcsicmp(entry.szModule, module_name)) {
			base_address = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
		}
		else {
			while (Module32Next(snapshot, &entry)) {
				if (!_wcsicmp(entry.szModule, module_name)) {
					base_address = reinterpret_cast<std::uintptr_t>(entry.modBaseAddr);
					break;
				}
			}
		}
	}
	CloseHandle(snapshot);
	return base_address;
}
