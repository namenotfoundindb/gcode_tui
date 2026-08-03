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
#include <vector>
#include <fstream>

std::string extract_first_word(std::string* str);

//Separate key=value pairs by spaces (in the future it will also separate by
//newlines)
std::vector<std::string> separate_key_value_pairs(std::string);

//takes an key-argument pair as a string and splits it into an std::pair by a
//: (colon)
//NOTE: It does not expect the argument to be surounded in quoates
std::pair<std::string, std::string> get_key_argument(std::string pair);

//Returns -1 on error
int count_file_lines(std::ifstream* file);
