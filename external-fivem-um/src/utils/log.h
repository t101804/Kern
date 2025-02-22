#include <string>
#include <ostream>
#include <iostream>
#include "color.hpp"

namespace Logging {
	void info_print(const std::string& value);
	void debug_print(const std::string& value);
	void error_print(const std::string& value);
}

