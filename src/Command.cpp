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

#include <iostream>
#include <string>

#include "Command.hpp"
#include "Commands.hpp"

#include "UserCommand.hpp"

int try_execute_Command(Command& cmd, CommandContext& context) {
	if (context.usrcmd.arguments.size() < cmd.required_arguments.size()) {
		std::cout << "Not enough arguments!" << std::endl;
		return -1;
	}

	return cmd.action(context);
}

int find_and_execute_Command(CommandContext& context) {
	std::string command_to_find = context.usrcmd.command;
	Command cmd;
	try {
		cmd = Commands::list.at(command_to_find);
	} catch (std::out_of_range& e) {
		return -1;
	}

	return try_execute_Command(cmd, context);
}

bool Command_exists(std::string cmdname) {
	return (bool) Commands::list.count(cmdname);
}
