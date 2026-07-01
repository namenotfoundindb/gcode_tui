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
#include <sstream>
#include <map>

class StringCommand {
	public:
		std::string data;
		std::string command;
		std::map<std::string, std::string> arguments;

		StringCommand(std::string str) {
			data = str;
			//go trough the string and find key-value pairs like:
			//send file:/home/casi/test.gcode
			//^-command
			//      ^-argument key
			//               ^-argument value

			std::stringstream ss(data);
			//The first word is the command itselft
			ss >> command;

			std::string word;

			while (ss >> word) {
				int i;
				for (i = 0; i < int(word.length()); i++) {
					//If we found the  delimiter
					if (word[i] == ':') break;
				}
				
				std::string key, value;
				key = word.substr(0, i);
				//i + 1 to skip over ':'
				value = word.substr(i + 1, word.length() - 
						i - 1);

				arguments.insert({key, value});
			}
		}

		void print() {
			std::cout << "command: " << command << std::endl;

			for (auto argument : arguments) {
				std::cout << argument.first << ":" 
					<< argument.second << std::endl;
			}
		}
};
