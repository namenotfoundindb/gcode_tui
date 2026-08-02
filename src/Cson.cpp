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

#include <stdexcept>
#include <string>
#include <vector>
#include <utility>
#include <map>

#include "string_functions.h"

#include "Cson.h"

//parse a cson string
//returns the number of values
int Cson::parse(std::string str) {
	data = str;

	std::vector<std::string> str_pairs = separate_key_value_pairs(str);
	std::pair<std::string, std::string> pair;

	//clear the previous values
	values.clear();

	for (std::string str_pair : str_pairs) {
		 pair = get_key_argument(str_pair);
		 values.insert(pair);
		 argc++;
	}

	return 0;
}

bool Cson::get(std::string key, std::string* dest) {
	try {
		*dest = values.at(key);
	} catch (const std::out_of_range& ex) {
		return false;
	}
	return true;
}
