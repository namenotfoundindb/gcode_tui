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

//Separate key=value pairs by spaces (in the future it will also separate by
//newlines)
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
			if (str[last_quote_pos + 1] == ' ')
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
