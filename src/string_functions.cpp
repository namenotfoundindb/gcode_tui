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
#include <vector>
#include <utility>
#include <fstream>

//move the first word from one string to another
//(and also delete the space/newline in the source string)
std::string extract_first_word(std::string* str) {
	//find the first space
	size_t newline_pos = str->find('\n');
	size_t space_pos = str->find(' ');
	std::string return_str;

	//separate only by the first separator
	size_t first_separator;
	if (space_pos < newline_pos) first_separator = space_pos;
	else first_separator = newline_pos;

	//if there is no separator
	if (first_separator == std::string::npos) {
		//return the whole string
		return_str = *str;
		*str = "";
	}
	else {
		return_str = str->substr(0, first_separator);
		*str = str->substr(first_separator + 1);
	}

	return return_str;
}

//Separate key=value pairs
std::vector<std::string> separate_key_value_pairs(std::string str) {
	std::vector<std::string> pairs;
	int delimiter_pos = 0;
	int last_quote_pos = 0;
	bool has_quotes;

	while (str.length() != 0) {
		std::string key_argument_pair;
		delimiter_pos = str.find(':');

		has_quotes = str[delimiter_pos + 1] == '\"';

		if (has_quotes) {
			//delete the first quote
			str.erase(delimiter_pos + 1, 1);

			//+2 to skip over the first quote
			last_quote_pos = str.find('\"', 
					delimiter_pos + 2);

			//save the key argument pair
			key_argument_pair = str.substr(0, 
					last_quote_pos);

			//delete this part of the string, and
			//leave the rest for the next iteration

			//check if the last part of str contains a space
			char char_after_quoate = str[last_quote_pos + 1];
			if (char_after_quoate == ' '
					|| char_after_quoate == '\n')
				//+2 to delete the last quoate and space
				str.erase(0, last_quote_pos + 2);

			//else +1 to delete only the quote
			else str.erase(0, last_quote_pos + 1);
		}

		else {
			key_argument_pair = extract_first_word(&str);
		}

		pairs.push_back(key_argument_pair);
	}

	return pairs;
}

//takes an key-argument pair as a string and splits it into an std::pair by a
//: (colon)
//NOTE: It does not expect the argument to be surounded in quoates
std::pair<std::string, std::string> get_key_argument(std::string pair) {
	int delimiter_pos = pair.find(":");
	std::string key = pair.substr(0, delimiter_pos);

	//delimiter_pos + 1 to skip over the delimiter
	std::string argument = pair.substr(delimiter_pos + 1, pair.length());

	return {key, argument};
}

//Returns -1 on error
int count_file_lines(std::ifstream* file) {
	if (!file->is_open()) return -1;

	int line_count;
	std::string temp;
	while (std::getline(*file, temp)) {
		line_count++;
	}

	return line_count;
}
