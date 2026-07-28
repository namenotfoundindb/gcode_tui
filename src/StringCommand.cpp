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

/* StringCommand is a class that gives the programmer funcntions to deal
 * with commands of the following type:
 * send file:test.gcode baudrate:115200 random_argument:69
 *
 * It makes it easy to acces the arguments. Say we want to get the 
 * baudrate_argument from the StringCommand object command:
 * std::string badurate = command.arguments["baudrate"];
 */

#include <iostream>
#include <string>
#include <map>
#include <vector>

#include "StringCommand.h"
#include "string_functions.h"

//Parse a string into the StringCommand object
int StringCommand::parse(std::string str) {
	//go trough the string and find key-value pairs like:
	//send file:/home/casi/test.gcode
	//^-command
	//      ^-argument key
	//               ^-argument value
	
	//if there is a newline at the end, erase it
	if (str[str.length() - 1] == '\n') str.erase(str.length() - 1, 1);

	data = str;
	command = extract_first_word(&str);

	if (str == "") return 0;

	//vector to store the key argument pairs
	//temporaraly
	std::vector<std::string> key_argument_pairs;

	int delimiter_pos = 0;
	int last_quote_pos = 0;

	bool has_quotes = false;

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

			//check if the last part contains a space
			if (str[str.length() - 1] == ' ') 
				//+2 to delete the last quoate and space
				str.erase(0, last_quote_pos + 2);
			else str.erase(0, last_quote_pos + 1);
		}

		else {
			key_argument_pair = extract_first_word(
					&str);
		}

		key_argument_pairs.push_back(key_argument_pair);
	}
	
	//string to store the values temporaraly
	std::string key;
	std::string argument;

	//empty it of the previous arguments
	arguments.clear();
	
	for (auto key_argument_pair : key_argument_pairs) {
		delimiter_pos = key_argument_pair.find(':');

		//get the key from the string
		key = key_argument_pair.substr(0, 
				delimiter_pos);

		//get the argument from the string
		//start at delimiter_pos +1 to skip over the 
		//delimiter
		argument = key_argument_pair.substr(
				delimiter_pos + 1,
				key_argument_pair.length());

		arguments.insert({key, argument});
	}

	return 0;
}

void StringCommand::print() {
	std::cout << "command: " << command << std::endl;

	for (auto argument : arguments) {
		std::cout << argument.first << ":" 
			<< argument.second << std::endl;
	}
}
