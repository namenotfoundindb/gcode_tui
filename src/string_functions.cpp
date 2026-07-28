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

//move the first word from one string to another
//(and also delete the space in the source string)
std::string extract_first_word(std::string* str) {
	//find the first space
	size_t pos = str->find(' ');
	std::string sub;

	//if there is a space in the string
	if (pos != std::string::npos) {
		//make a substring containing the first word
		sub = str->substr(0, pos);

		//assign the rest of the string to the original string
		*str = str->substr(pos + 1);
	}

	//else if the string contains one word (no spaces);
	else {
		//return the original string
		sub = *str;
		*str = "";
	}
	return sub;
}
