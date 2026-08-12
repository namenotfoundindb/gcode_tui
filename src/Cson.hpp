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

#include <string>
#include <map>

class Cson {
	public:
		//value count
		int argc = 0;
		std::map<std::string, std::string> values;
		std::string data;

		int parse(std::string str);

		//get a value
		//The value is copied into dest
		//Returns true if it found the value, false if it does not
		//exist
		//NOTE: I know this is a very C way of doing it
		bool get(std::string key, std::string* dest);
};
