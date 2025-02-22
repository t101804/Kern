#include "log.h"

void Logging::info_print(const std::string& value) {
	std::cout
		<< termcolor::white << "["         // White for "["
		<< termcolor::green << "+"         // Green for "+"
		<< termcolor::white << "] "        // White for "] "
		<< termcolor::yellow << "INFO "    // Yellow for "INFO "
		<< termcolor::white << ": "        // White for ": "
		<< termcolor::white << value       // White for the value
		<< termcolor::reset << std::endl;  // Reset colors for subsequent output
}

void Logging::debug_print(const std::string& value) {
	std::cout
		<< termcolor::white << "["         // White for "["
		<< termcolor::blue << "*"          // Blue for "*"
		<< termcolor::white << "] "        // White for "] "
		<< termcolor::yellow << "DEBUG "   // Yellow for "DEBUG "
		<< termcolor::white << ": "        // White for ": "
		<< termcolor::white << value       // White for the value
		<< termcolor::reset << std::endl;  // Reset colors for subsequent output
}

void Logging::error_print(const std::string& value) {
	std::cout
		<< termcolor::white << "["         // White for "["
		<< termcolor::red << "-"           // Red for "-"
		<< termcolor::white << "] "        // White for "] "
		<< termcolor::yellow << "ERROR "   // Yellow for "ERROR "
		<< termcolor::white << ": "        // White for ": "
		<< termcolor::white << value       // White for the value
		<< termcolor::reset << std::endl;  // Reset colors for subsequent output
}