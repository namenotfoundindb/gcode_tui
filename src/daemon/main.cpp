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
#include <fstream>
#include <chrono>

#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/stat.h>	//for umask()
#include <sys/socket.h>
#include <termios.h>

#include "../commons.h"
#include "daemon_functions.h"
#include "StringCommand.h"
#include "Printer.h"

//#define DONT_CHDIR

const short int formatted_time_length = 32;

std::string log_file_path = "/var/log/gcode_tui_daemon.log";
std::ofstream log_file;

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
  help - display this help message\n\
  echo - print out text\n\
  init - initilize the printer\n\
  terminal - connect to the printer with a terminal\n\
  exit - disconnect from the daemon\n\
  shutdown - shutdown the daemon\n\
\n\
";

int client_socket;
int client;

void log(std::string text) {
	time_t timestamp;
	time(&timestamp);

	//ctime returns a pointer to a nul terminated strings that contains
	//a newline before the null terminator. here i am removing the neline
	char* time_string = ctime(&timestamp);

	//Put a null terminator where the newline was (str_length - 1)
	time_string[strlen(time_string) - 1] = '\0';
	
	log_file << time_string << ": " << text << std::endl;
}

int terminal_loop(Printer printer, int client) {
	char* buffer = ( char* ) malloc(BUFFER_SIZE);
	ssize_t read_bytes;

	while (true) {
		write(client, "SEND: ", 6);
		read_bytes = read(client, buffer, BUFFER_SIZE);
	
		//add the null terminator
		buffer[read_bytes] = '\0';

		if (strcmp(buffer, "exit\n") == 0) { free(buffer); return 0; }

		printer.send((std::string{buffer}));

		do {
			read_bytes = printer.read_line(buffer);

			if (read_bytes < 0) {
				 write(client, 
"ERROR reading printer's response.\n", 35);
				free(buffer);
				return -1;
			}

			write(client, "RECV: ", 6);
			write(client, buffer, strlen(buffer));
		} while (!printer.is_response_ok(buffer) && read_bytes == 0);
	}

	free(buffer);
	return 0;
}

int client_loop() {
	char* buffer = ( char* ) malloc(BUFFER_SIZE);
	strcpy(buffer, "Hello client! This is gcode_tui_daemon!\n");
	ssize_t read_bytes = 0;
	Printer printer;
	

	while (true) {
		client = accept(client_socket, nullptr, nullptr);

		if (client < 0) {
			log("ERROR accepting client!");
			return -1;
		}

		else log("Accepted client");

		while (true) {
			read_bytes = read(client, buffer, BUFFER_SIZE);

			if (read_bytes < 0) {
				log("ERROR reading from the client!");
				free(buffer);
				return -1;
			}

			//read does not put a null terminator at the end, so i 
			//put it myself
			buffer[read_bytes] = '\0';

			StringCommand client_command;
			if (client_command.parse({std::string(buffer)}) != 0) {
				write(client, "ERROR parsing command!\n", 22);
			}	

			else if (client_command.command == "exit") {
				close(client);
				log("Ended connection with client");
				break;
			}

			else if (client_command.command == "shutdown") {
				log("Shuting down gcode_tui_daemon...");
				free(buffer);
				close(client);
				printer.disconnect();
				return 0;
			}

			else if (client_command.command == "help") 
				write(client, help_text.c_str(),
					help_text.length());

			else if (client_command.command == "echo") {
				write(client,
					client_command.arguments["text"].c_str(),
					client_command.arguments["text"].length());
				write(client, "\n", 1);
			}

			else if (client_command.command == "terminal") {
				write(client, 
"You have entered terminal mode, type \"exit\" to go back.\n", 56);

				log("Client entered terminal mode");

				terminal_loop(printer, client);
				write(client, "Exited terminal mode\n", 21);

				log("Client left terminal mode");

			}

			else if (client_command.command == "init") {
				//at the moment the baudrate is fixed
				//and there is no check to make sure the 
				//argument "printer" exists
				printer.init(client_command.arguments["printer"]
						, B115200);
				write(client, "Initilized printer\n", 19);
			}

			else write(client, "Unknown command! Try \"help\"\n",
					28);

			log("Sent message to client");
			sleep(1);
		}

	}	
	close(client);
	free(buffer);

	return 0;
}

int main() {
	ssize_t error_num = 0;
	error_num = daemonize();

	if (error_num < 0) {
		std::cout << "ERROR daemonizing!" << std::endl;
		return errno;
	}

	log_file.open(log_file_path);

	log("Started gcode_tui daemon");

	client_socket = init_socket();
	if (client_socket < 0) {
		log("ERROR initializing client socket!");
		return client_socket;
	}
	
	client_loop();

	log("Exiting...\nThank you for using gcode_tui!");

	unlink(daemon_socket_addres.c_str());
	log_file.close();

	std::cout << "Hello world!" << std::endl;
	return 0;
}
