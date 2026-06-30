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

#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <sys/un.h>
#include <sys/stat.h>	//for umask()

std::string log_file_path = "gcode_tui_daemon.log";
std::ofstream log_file;

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

void log(std::string text) {
	time_t timestamp;
	time(&timestamp);
	log_file << ctime(&timestamp) << ": " << text << std::endl;
}

int main_loop() {
	return 0;
}

int main() {
	ssize_t error_num;
	error_num = daemonize();

	if (error_num < 0) {
		std::cout << "ERROR daemonizing!" << std::endl;
		return errno;
	}

	log_file.open(log_file_path);


	std::cout << "Hello world!" << std::endl;
	return 0;
}
