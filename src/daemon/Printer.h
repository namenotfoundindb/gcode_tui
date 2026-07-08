/*
    gcode_tui: Program that drips gcode in the background to a machine
    Copyright (C) 2026 namenotfoundindb

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 3 as 
    published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#pragma once

#include <stdint.h>
#include <string>
#include <termios.h>
#include <time.h>

#include <queue>
#include <condition_variable>

#include "PrinterCommands.h"
#include "PrinterState.h"

class Printer {
	public:
		bool initialized = false;
		std::string path;
		int fd;
		struct termios serial_settings;

		//at the moment public for testing
		std::queue<PrinterCommands> command_queue;	
		PrinterState state = PrinterState::Idle;

		//how much time to wait for data if there is none from the 
		//printer
		//at the moment these are the default values
		struct timespec wait_for_data_delay = {
			//0 seconds
			.tv_sec = 0,
			//and 25 miliseconds
			.tv_nsec = 25000000
		};

		int read_garbage();
		int init(std::string path, uint64_t badurate);
		int send(std::string gcode);
		int send_file(std::ofstream gcode_file);
		ssize_t read_char(char* ch);
		int read_line(char* buffer);
		bool is_response_ok(char* buffer);
		void disconnect();
};
