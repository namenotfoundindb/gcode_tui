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

#include <string>
#include <fstream>
#include <string.h>
#include <chrono>

#include "log.hpp"

std::string log_file_path = "/var/log/gcode_tui_daemon.log";
std::ofstream log_file;

void log(std::string text) {
	time_t timestamp;
	time(&timestamp);

	//ctime returns a pointer to a nul terminated strings that contains
	//a newline before the null terminator. here i am removing the neline
	char* time_string = ctime(&timestamp);

	//Put a null terminator where the newline was (str_length - 1)
	time_string[strlen(time_string) - 1] = '\0';
	
	log_file << time_string << ": " << text << std::endl;
}
