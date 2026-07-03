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

int main_loop(Printer printer) {
	ssize_t error_num;
	std::string send_buffer;
	char* buffer = ( char* ) malloc(BUFFER_SIZE);

	while (true) {
		std::cout << "SEND: ";
		std::cin >> send_buffer;
		if (send_buffer == "exit") { return 0; free(buffer); }
		send_buffer.append(1, '\n');

		printer.send(send_buffer);

		do {
			error_num = printer.read_line(buffer);

			if (error_num < 0) {
				std::cout << "ERROR reading printer's response: "
					<< error_num << std::endl;
				free(buffer);
				return error_num;
			}

			std::cout << "RECV: " << buffer;
		} while (!printer.is_response_ok(buffer) && error_num == 0);
	}

	free(buffer);
	return 0;
}

int client_loop() {
	char* buffer = ( char* ) malloc(BUFFER_SIZE);
	strcpy(buffer, "Hello client! This is gcode_tui_daemon!\n");
	int error_num = 0;
	ssize_t read_bytes = 0;

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

			if (strcmp(buffer, "exit\n") == 0) {
				log("Ended connection with client");
				free(buffer);
				return 0;
			}

			//echo back what the client sent
			if (write(client, buffer, strlen(buffer)) < 0) {
				log("ERROR writing to the client!");
				free(buffer);
				return -1;
			}

			log("Sent message to client");
			sleep(1);
		}

		close(client);
		log("Ended connection with client");
		free(buffer);

		return 0;
	}	
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
	
	Printer printer("/dev/ttyUSB0", B115200);
	
	client_loop();

	close(printer.fd);

	log("Exiting...\nThank you for using gcode_tui!");

	unlink(daemon_socket_addres.c_str());
	log_file.close();

	std::cout << "Hello world!" << std::endl;
	return 0;
}
