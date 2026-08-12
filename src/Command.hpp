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

#include <vector>
#include <functional>
#include <string>

#include "UserCommand.hpp"

//struct containg info (or *context*) that a command might need when executing
//in the future will contain client file descriptors, a pointer to a Printer and
//so on
struct CommandContext {
	UserCommand& usrcmd;
};

//info about a command. Probably could make it's members constant
struct Command {
	std::vector<std::string> required_arguments;
	std::function<int(CommandContext&)> action;
	std::string description;
};
