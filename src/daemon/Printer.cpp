#include <string>

#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <unistd.h>

#include "../commons.h"
#include "Printer.h"

Printer::Printer(std::string path, uint64_t baudrate) {
	//Initialize the serial_settings
	fd = open(path.c_str(), O_RDWR | O_NDELAY);

	//Wait a bit for printer to initialize
	sleep(2);

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

	//Read all the garbage that the printer says
	char* buffer = ( char* ) malloc(BUFFER_SIZE);
	ssize_t error_num;
	do {
		error_num = read(fd, buffer, BUFFER_SIZE);
	} while (error_num != EAGAIN && error_num > 0);
}

ssize_t Printer::send(std::string gcode) {
	return write(fd, gcode.c_str(), gcode.length());
}
