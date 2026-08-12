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

#include <map>

#include "Command.hpp"

#define cmdentry(cmdname) {#cmdname, Commands::cmdname}

namespace Commands {
	//Corespoding functions to commands
	namespace Functions {
		int echo(CommandContext& context);
		int help(CommandContext& context);
	}

	//Actual commands
	extern Command echo;

	//A map of all to commands
	const std::map<std::string, Command&> list = {
		cmdentry(echo)
	};
}

//Should only be called by find_and_execute_command
int try_execute_Command(Command& cmd, CommandContext& context);
int find_and_execute_Command(CommandContext& context);
