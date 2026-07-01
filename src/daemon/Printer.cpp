#include <string>

#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <unistd.h>

#include "../commons.h"
#include "Printer.h"

void Printer::read_garbage() {
	//Read all the garbage that the printer says, until an 
	//EAGAIN, thats when there is no more data to read
	char* buffer = ( char* ) malloc(BUFFER_SIZE);
	ssize_t error_num;
	do {
		error_num = read(fd, buffer, BUFFER_SIZE);
	} while (error_num != EAGAIN && error_num > 0);
	free(buffer);
}

Printer::Printer(std::string path, uint64_t baudrate) {
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

}

ssize_t Printer::send(std::string gcode) {
	return write(fd, gcode.c_str(), gcode.length());
}
