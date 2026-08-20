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
#include "daemon/Commands.hpp"

#include "UserCommand.hpp"

#include <unistd.h>

int try_execute_Command(Command& cmd, CommandContext& context) {
	if (context.usrcmd.arguments.size() < cmd.required_arguments.size()) {
		write(context.client, "Not enough arguments!\n", 23);
		return ReturnNotEnoughArgs;
	}

	for (std::string required_arg : cmd.required_arguments) {
		if (!context.usrcmd.arguments.count(required_arg)) {
			write(context.client, ((std::string) 
					("Argument \"" + required_arg +
					 "\" needed but not found!\n")).c_str(),
					35 + required_arg.length());
			return ReturnNotEnoughArgs;
		}
	}

	return cmd.action(context);
}

int find_and_execute_Command(CommandContext& context) {
	std::string command_to_find = context.usrcmd.command;
	Command cmd;
	try {
		cmd = Commands::list.at(command_to_find);
	} catch (std::out_of_range& e) {
		write(context.client, "Command not found! Try \"help\"\n", 31);
		return ReturnCommandNotFound;
	}

	return try_execute_Command(cmd, context);
}

bool Command_exists(std::string cmdname) {
	return (bool) Commands::list.count(cmdname);
}
