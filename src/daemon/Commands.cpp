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

#include <stdexcept>
#include <unistd.h>

#include "Commands.hpp"
#include "../Command.hpp"

#include "int_to_termios_baudrate.hpp"

//Actual command defintions and functions

int Commands::Functions::echo(CommandContext& context) {
	auto text = context.usrcmd.get_arg("text");

	write(context.client, ((std::string) (text.value() + "\n")).c_str(),
			text.value().length() + 1);
	return 0;
}

Command Commands::echo = {
	.required_arguments = {"text"},
	.action = Commands::Functions::echo,
	.description = "Print text to the console"
};


int Commands::Functions::help(CommandContext& context) {
	std::string help_text = "\
gcode_tui: Program that drips gcode in the background to a machine\n\
Copyright (C) 2026 namenotfoundindb\n\
\n\
COMMANDS AND ARGUMENTS:\n\
Commands work like you think the do, just type them in!\n\
Arguments work by typing the argument name, a \':\' and then the\n\
value. If the value contains spaces, put the whole value in quotes.\n\
EXAMPLES:\n\
echo text:Hello\n\
echo text:\"Hello world!\"\n\
\n\
SUPPORTED COMMANDS:\n\
";

	for (auto cmd : Commands::list) {
		help_text.append(cmd.first + " - " + cmd.second.description + 
				"\n");
	}

	help_text.append("\n");

	write(context.client, help_text.c_str(), help_text.length());
	return 0;
}

Command Commands::help = {
	.required_arguments = {},
	.action = Commands::Functions::help,
	.description = "Print help about commands"
};

int Commands::Functions::exit(CommandContext& context) {
	write(context.client, "Goodbye.\n", 10);
	return ActionDisconnectClient;
}

Command Commands::exit = {
	.action = Commands::Functions::exit,
	.description = "Disconnect from the daemon"
};

int Commands::Functions::shutdown(CommandContext& context) {
	write(context.client, "Shuting down...\n", 17);
	return ActionShutdown;
}

Command Commands::shutdown = {
	.action = Commands::Functions::shutdown,
	.description = "Shutdown gcode_tui_daemon"
};


int Commands::Functions::init(CommandContext& context) {
	int baudrate = std::stoul(context.usrcmd.arguments["baudrate"]);
	int termios_baudrate;
	try {
		termios_baudrate = to_termios_baudrate.at(50);
	} catch (std::out_of_range& e) {
		write(context.client, "ERROR: No such baudrate!\n", 26);
		return ReturnCommandErrored;
	}

	if (context.printer.init(context.usrcmd.arguments["printer"],
				termios_baudrate) == ReturnSuccesful) {
		write(context.client, "Initilized printer!\n", 21);
	}

	//i don't think Printer::init can return -1, but il stil put this here
	else {
		write(context.client, "ERROR: Initilizing printer!\n", 29);
	}

	return 0;
}

Command Commands::init = {
	.required_arguments = {"printer", "baudrate"},
	.action = Commands::Functions::init,
	.description = "Initilaize a printer"
};
