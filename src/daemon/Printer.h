#pragma once

#include <stdint.h>
#include <string>
#include <termios.h>

class Printer {
	public:
		std::string path;
		int fd;
		struct termios serial_settings;

		Printer(std::string path, uint64_t badurate);
		ssize_t send(std::string gcode);
};
