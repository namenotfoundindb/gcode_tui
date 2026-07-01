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
#include <signal.h>
#include <sys/un.h>
#include <sys/stat.h>	//for umask()
#include <sys/socket.h>

#include "../commons.h"

//#define DONT_CHDIR

const short int formatted_time_length = 32;

#ifdef DONT_CHDIR
std::string log_file_path = "gcode_tui_daemon.log";
#else 
std::string log_file_path = "/var/log/gcode_tui_daemon.log";
#endif

std::ofstream log_file;

int client_socket;
int client;

int daemonize() {
	signal(SIGPIPE, SIG_IGN);
	pid_t pid;

	//Fork the parent
	if((pid	 = fork()) < 0) {
		return -1;
	}
	else if (pid != 0) {
		exit(0); //exit parent
	}

	//create new session
	if (setsid() < 0) {
		return -2;
	}

#ifndef DONT_CHDIR
	//change directory
	if(chdir("/") < 0) {
		return -3;
	}
#endif

	//reset file permissions
	umask(0);

	//close standard outputs
	close(STDIN_FILENO);
	close(STDERR_FILENO);
	close(STDOUT_FILENO);

	//redirect everything to /dev/null
	open("/dev/null", O_RDONLY);
	open("/dev/null", O_WRONLY);
	open("/dev/null", O_WRONLY);

	return 0;
}

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

int init_socket() {
	//create the socket
	int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);

	sockaddr_un addr{};

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, daemon_socket_addres.c_str());

	//unbind the previous socket (if there was any)
	unlink(daemon_socket_addres.c_str());

	//Bind the socket
	bind(client_socket, (sockaddr*)&addr, sizeof(addr));

	return client_socket;
}

int main_loop() {
	while (true) {
		
	}
}

int main() {
	ssize_t error_num;
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

	sleep(10);

	log("Exiting...\nThank you for using gcode_tui!");

	unlink(daemon_socket_addres.c_str());
	log_file.close();

	std::cout << "Hello world!" << std::endl;
	return 0;
}
