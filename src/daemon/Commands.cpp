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

#include <string.h>

#include "Commands.hpp"
#include "../Command.hpp"

#include "int_to_termios_baudrate.hpp"
#include "../string_functions.hpp"
#include "log.hpp"
#include "Printer/Commands.hpp"
#include "Printer/State.hpp"

#include "../commons.hpp"

//Some helper functions
void send_command(PrinterCommands cmd, CommandContext& context) {
	{
		std::lock_guard<std::mutex> lock(context.mtx);
		context.printer.command_queue.push(cmd);
	}

	//notify the gcode sender thread that a command has been pushed
	context.cv.notify_one();
}

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
		termios_baudrate = to_termios_baudrate.at(baudrate);
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

int Commands::Functions::send(CommandContext& context) {
	std::unique_lock<std::mutex> lock(context.mtx);
	//NOTE: global_printer is NULL, so don't try to acces it.
	//BTW this isn't here because i did just that, no way.

	if (context.printer.state == Uninitialized) {
		write(context.client, "ERROR: Printer is uninitilized!\n", 33);
		return ReturnCommandErrored;
	}

	context.printer.file_to_send = context.usrcmd.arguments["file"];
	lock.unlock();

	context.printer.gcode_file.open(context.printer.file_to_send);

	if (!context.printer.gcode_file.is_open()) {
		write(context.client, "ERROR: Could not open file!\n", 22);
		return ReturnCommandErrored;
	}

	lock.lock();

	//i know this is ineficient because
	//count_file_lines opens the gcode_file again.
	//If you are more interested check out the lines
	//above count_file_lines' definition in
	//../string_functions.cpp
	context.printer.total_gcode_lines = count_file_lines(
			context.printer.file_to_send);

	context.printer.lines_proccesed = 0;
	context.printer.percentage_sent = 0;

	//set global_printer at the end, so that the
	//gcode_thread does not start sending gcode to
	//early
	context.global_printer = &context.printer;
	lock.unlock();

	//no need to call cv.notify_one because send
	//command already does that
	send_command(Start, context);

	log("Started sending file");
	return 0;
}

Command Commands::send = {
	.required_arguments = {"file"},
	.action = Commands::Functions::send,
	.description = "Send gcode file to the printer"
};


int Commands::Functions::terminal(CommandContext& context) {
	write(context.client,
		"You have entered terminal mode, type \"exit\" to go back.\n",
		56);
	log("Client entered terminal mode");

	char* buffer = ( char* ) malloc(BUFFER_SIZE);
	ssize_t read_bytes;

	while (true) {
		write(context.client, "SEND: ", 6);
		read_bytes = read(context.client, buffer, BUFFER_SIZE);

		//add the null terminator
		buffer[read_bytes] = '\0';

		if (strcmp(buffer, "exit\n") == 0) break;

		context.printer.send((std::string{buffer}));

		do {
			read_bytes = context.printer.read_line(buffer);

			if (read_bytes < 0) {
				 write(context.client,
"ERROR reading printer's response.\n\
 Exited terminal mode.\n",58);
				 free(buffer);
				 return ReturnCommandErrored;
			}

			write(context.client, "RECV: ", 6);
			write(context.client, buffer, strlen(buffer));

		} while (!context.printer.is_response_ok(buffer));
	}
	free(buffer);

	write(context.client, "Exited terminal mode\n", 21);
	log("Client left terminal mode");

	return 0;
}

Command Commands::terminal = {
	.required_arguments = {},
	.action = Commands::Functions::terminal,
	.description = "Send/Receive to/from the printer (only for testing)"
};

int Commands::Functions::status(CommandContext& context) {
	//calculate the percentage sent first
	{
		std::lock_guard<std::mutex> lock(context.mtx);
		context.printer.percentage_sent = (int) (
				( (float) context.printer.lines_proccesed /
				  (float) context.printer.total_gcode_lines)
				* 100.0f);
	}

	Cson cson_info = context.printer.get_cson_status();
	write(context.client, cson_info.data.c_str(), cson_info.data.length());
	return 0;
}

Command Commands::status = {
	.required_arguments = {},
	.action = Commands::Functions::status,
	.description = "Get status about the printer/print job"
};


int Commands::Functions::continue_print(CommandContext& context) {
	PrinterState pstate;
	{
		std::lock_guard<std::mutex> lock(context.mtx);
		pstate = context.printer.state;
	}

	if (pstate == Errored || pstate == Paused) {
		send_command(Continue, context);

		if (pstate == Errored) {
			write(context.client, "Printer error cleared. ", 24);
		}

		if (pstate == Paused) {
			write(context.client, "Continuing...\n", 15);
		}
	}

	else write(context.client, "Nothing to continue!\n", 22);

	return 0;
}

Command Commands::continue_print = {
	.required_arguments = {},
	.action = Commands::Functions::continue_print,
	.description = "Continue printing after a pause/error"
};
