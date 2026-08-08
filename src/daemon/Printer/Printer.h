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

#include "Commands.h"
#include "State.h"
#include "../../Cson.h"

class Printer {
	public:
		std::queue<PrinterCommands> command_queue;

		struct termios serial_settings;

		//how much time to wait for data if there is none from the 
		//printer
		//at the moment these are the default values
		struct timespec wait_for_data_delay = {
			//0 seconds
			.tv_sec = 0,
			//and 25 miliseconds
			.tv_nsec = 25000000
		};

		std::string path;
		std::string file_to_send;

		unsigned int lines_proccesed;
		unsigned int total_gcode_lines;

		int fd;
		short int percentage_sent;
		PrinterState state = PrinterState::Uninitialized;

		int read_garbage();
		int init(std::string path, uint64_t termios_badurate);
		int send(std::string gcode);
		ssize_t read_char(char* ch);
		int read_line(char* buffer);
		bool is_response_ok(char* buffer);
		void disconnect();
		Cson get_cson_status();
		bool should_line_be_sent(std::string line);
};
