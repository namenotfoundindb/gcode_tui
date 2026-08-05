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

#include <string>

#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>

#include <fstream>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "../../commons.h"
#include "Printer.h"
#include "../../Cson.h"
#include "State.h"

int Printer::read_garbage() {
	//Read all the garbage that the printer says, until an 
	//EAGAIN, thats when there is no more data to read
	char* buffer = ( char* ) malloc(BUFFER_SIZE);
	int error_num = 0;
	do {
		if (read(fd, buffer, 1) < 0) {
			error_num = errno;
			if (error_num != EAGAIN) {
				free(buffer);
				return -1;
			}
		}

	} while (error_num != EAGAIN);
	free(buffer);
	return 0;
}

int Printer::init(std::string path, uint64_t baudrate) {
	Printer::path = path;

	//Initialize the serial_settings
	fd = open(path.c_str(), O_RDWR | O_NDELAY);

	//Wait a bit for printer to initialize
	sleep(1);

	//Get serial port configuration
	tcgetattr(fd, &serial_settings);

	//Enale NON-CONONICAL mode
	serial_settings.c_lflag &= ~(ICANON | ECHO | ECHOE | 
			ISIG);

	//Turn off software flow control
	serial_settings.c_iflag &= ~(IXON | IXOFF | IXANY);

	//Deactivate CR to NL translation
	serial_settings.c_iflag &= ~ICRNL;

	//Clear output proccesing
	serial_settings.c_oflag &= ~OPOST;

	//Turn off hardware based flow-control
	serial_settings.c_cflag &= ~CRTSCTS;

	//Turn on receiver
	serial_settings.c_cflag |= CREAD | CLOCAL;

	serial_settings.c_cflag &= ~PARENB;	//No parity bit
	serial_settings.c_cflag &= ~CSTOPB;	//One stop bit
	serial_settings.c_cflag &= ~CSIZE;
	serial_settings.c_cflag |= CS8;	//8 bits per character

	//Set baud rate
	cfsetispeed(&serial_settings, baudrate);
	cfsetospeed(&serial_settings, baudrate);

	//update settings
	//TCSANOW means update them now
	tcsetattr(fd, TCSANOW, &serial_settings); 		

	read_garbage();

	//i don't know why, but the first command that i send to the printer
	//allways ends up erroring out.
	//to counter this i send this before anything else
	write(fd, "M117 Hello from gcod_tui!\n", 26);
	//M117 puts a message on the printers screen

	//if the command works, then we got a message on the screen
	//if it does not, at least we sent the first command and got an error
	//now and not later
	
	//wait a bit for the printer to say is's message
	sleep(1);
	read_garbage();
	
	//change the printer state from Unitialized
	state = Idle;

	return 0;
}

int Printer::send(std::string gcode) {
	return write(fd, gcode.c_str(), gcode.length());
}

ssize_t Printer::read_char(char* ch) {
	int error_num;

	do {
		if (read(fd, ch, 1) == -1) {
			error_num = errno;
			if (error_num != EAGAIN) return -1;
		
			//if there is no data (ie errno == EAGAIN) wait for some
			else if (error_num == EAGAIN) 
				nanosleep(&wait_for_data_delay, NULL);
		}

		else error_num = 0;
	} while (error_num == EAGAIN);
	return 0;
}

int Printer::read_line(char* buffer) {
	char ch;
	int strlen = 0;

	buffer[0] = '\0';

	while (true) {
		//read one character at a time
		if (read_char(&ch) < 0) return -1;

		//append them to the end of the buffer
		//NOTE: cannot use strcat as that function expects a null
		//terminated string. here we append a character not a string
		buffer[strlen] = ch;
		strlen++;

		if (ch == '\n') { buffer[strlen] = '\0'; return strlen; }
	}

}

//checks if the printers response contains 'ok' in the first two bytes
bool Printer::is_response_ok(char* buffer) {
	//only check the first two bytes because some gcode commands
	//send back a line containing ok and then a bunch of other stuff,
	//so we can't compare the whole line
	if (buffer[0] != 'o') return false;
	if (buffer[1] != 'k') return false;

	return true;
}

void Printer::disconnect() {
	close(fd);
	state = Uninitialized;
}

//get a string containing cson formatted information about the printer status
Cson Printer::get_cson_status() {
	//im sure this is not the best way to implement this functionality, but
	//im not going to spend 5 months finding the best way
	std::string str_info;

	str_info.append("state:");
	str_info.append(std::to_string(( int ) state) + "\n");

	if (state != Uninitialized) {
		//give more info if the printer is initilized
		str_info.append(
			"path:" + path + "\n" +
			"fd:" + std::to_string(static_cast<int>(fd)) +
			"\n");
	}

	if (state == Printing || state == Errored || state == Paused) {
		//if the printer is actualy printing
		str_info.append(
			"file_to_send:" + file_to_send + "\n" +
			"lines_proccesed:" + std::to_string(lines_proccesed) +
			"\n" +
			"percentage_sent:" + std::to_string(percentage_sent) +
			"\n" +
			"total_gcode_lines:" + std::to_string(total_gcode_lines)
			+ "\n");
	}

	Cson cson_info;
	cson_info.parse(str_info);

	return cson_info;
}
