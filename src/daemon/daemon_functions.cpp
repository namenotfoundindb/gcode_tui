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

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>	//for umask()
#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <string.h>

#include "../commons.hpp"

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

	//change directory
	if(chdir("/") < 0) {
		return -3;
	}

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

int init_socket() {
	//create the socket
	int client_socket = socket(AF_UNIX, SOCK_STREAM, 0);

	sockaddr_un addr{};

	addr.sun_family = AF_UNIX;
	strcpy(addr.sun_path, daemon_socket_addres.c_str());

	//unbind the previous socket (if there was any)
	unlink(daemon_socket_addres.c_str());

	//Bind the socket
	if (bind(client_socket, (sockaddr*)&addr, sizeof(addr)) < 0)
		return -1;

	listen(client_socket, 5);

	return client_socket;
}
