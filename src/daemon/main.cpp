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

/*
 * BUGS:
 * * You cannot stop the gcode_thread if global_printer = NULL because it will
 *   wait until global_printer != NULL. Will have to work on a better way of
 *   stoping the gcode_thread.
 */

#include <iostream>
#include <string>
#include <format>
#include <fstream>
#include <chrono>

#include <stdlib.h>

#include <fcntl.h>
#include <unistd.h>
#include <sys/un.h>
#include <sys/stat.h>	//for umask()
#include <sys/socket.h>
#include <termios.h>

#include <thread>
#include <mutex>
#include <condition_variable>

#include "../commons.h"
#include "daemon_functions.h"
#include "StringCommand.h"
#include "Printer/Printer.h"
#include "Printer/Commands.h"
#include "Printer/State.h"

//#define DONT_CHDIR

const short int formatted_time_length = 32;

std::string log_file_path = "/var/log/gcode_tui_daemon.log";
std::ofstream log_file;

//mutex between the gcode_thread and the main thread
std::mutex mtx;
//condition_variable between the gcode_thread and the main thread
std::condition_variable cv;

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
  send - sends a file to the printer\n\
  exit - disconnect from the daemon\n\
  shutdown - shutdown the daemon\n\
\n\
";

int client_socket;
int client;

//pointer to the active printer
//if set to NULL the gcode_thread waits until it is no longer NULL
Printer* global_printer = NULL;
std::string global_file_to_send = "";

bool end_gcode_thread = false;
uint lines_sent;

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

//TODO: continue writing the send_file function to a working state

//A temporary function to test multitheading and sending files
//In the future i plan to integrate this functions (and the 
//variables it uses) in the Printer class
int send_file() {
	std::unique_lock<std::mutex> lock(mtx);
	lock.lock();

	log("started the gcode_thread");

	if (!global_printer->initialized) return -1;
	lock.unlock();

	std::string line;
	int bytes_read = 0;
	char* buffer = ( char* ) malloc(BUFFER_SIZE);
	char* no_newline_buffer = ( char* ) malloc(BUFFER_SIZE);

	std::ifstream file;
	file.open(global_file_to_send);

	//put a lock on mtx so that this thread does not
	//acces/modify variables at the as the main thread

	while (true) {
		lock.lock();
		cv.wait(lock, [&] {return global_printer != NULL;});

		//wait until there are commands in the queue or
		//we are printing
		cv.wait(lock, [&] {return !global_printer->command_queue.empty()
				|| global_printer->state == PrinterState::Printing;
				});
		
		if (!global_printer->command_queue.empty()) {
			PrinterCommands cmd = global_printer->command_queue.front();
			global_printer->command_queue.pop();
			lock.unlock();

			if (cmd == PrinterCommands::Stop)
				global_printer->state = PrinterState::Stoped;
			else if (cmd == PrinterCommands::Start)
				global_printer->state = PrinterState::Printing;
			else if (cmd == PrinterCommands::Stop)
				global_printer->state = PrinterState::Stoped;
			else if (cmd == PrinterCommands::Pause)
				global_printer->state = PrinterState::Paused;
		}

		if (global_printer->state == PrinterState::Printing) {
			lock.lock();
			if (!getline(file, line)) {
				global_printer->state = PrinterState::Finished;
				log("Finished sending file");
			}

			 if (global_printer->send(line) < 0) {
				 global_printer->state = PrinterState::Errored;
				 log("ERROR sending line to printer!");
			 }
			 else log(std::format("SENT: {}", line));

			do {
				bytes_read = global_printer->read_line(buffer);
				if (bytes_read < 0) 
					global_printer->state = PrinterState::Errored;
				else {
					strcpy(no_newline_buffer, buffer);
					
					//delete the newline
					//-2 to reach the newline
					//h e l l o \n \0
					//                ^ here is strlen
					//           ^ here is strlen - 2
					no_newline_buffer[bytes_read - 2] = '\0';
					log(std::format("RECV: {}", buffer));
				}

			} while (!global_printer->is_response_ok(buffer) &&
					global_printer->state != PrinterState::Errored);
			lock.unlock();
		}

	}
	file.close();
	
	//make sure this_thread did not lock it forever
	lock.unlock();
	
	free(buffer);
	free(no_newline_buffer);

	return 0;
}

void send_command(PrinterCommands cmd) {
	//a separate scope so the lock_guard unlocks
	//when going out of scope
	{
		std::lock_guard<std::mutex> lock(mtx);
		global_printer->command_queue.push(cmd);
	}

	//notify the gcode sender thread that a command has been pushed
	cv.notify_one();
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
				free(buffer); return -1; }

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

			else if (client_command.command == "send") {
				{
					std::lock_guard<std::mutex> lock(mtx);
					global_printer = &printer;
					global_file_to_send =
						client_command.arguments["file"];
				}
				cv.notify_one();

				log("Started sending file");
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

int gcode_sender() {
	std::unique_lock<std::mutex> lock(mtx);

	char* buffer = (char*) malloc(BUFFER_SIZE);
	std::ifstream gcode_file;

	//if this line is empty ("") that means that the last line of gcode
	//was sent succesfuly, if it's not empty, try sending that line again
	std::string line = "h";

	//NOTE: In this outer loop, lock stays locked most of the time, while
	//in the inter loop it stays unlocked most of the time
	while (true) {
		if (end_gcode_thread) {
			lock.unlock();
			//break to the end of the function
			break;
		}

		//wait for the printer to be ready
		//when global_printer != NULL it means a file needs to be sent
		cv.wait(lock, [&] { return global_printer != NULL; });

		gcode_file.open(global_file_to_send);
		if (!gcode_file.is_open()) {
			//these error messages will be swaped in the future for
			//an error reporting system
			log("Error opening gcode file!");
			break;
		}

		//NOTE: In this inside loop, the lock remains unlocked most of
		//the time and locked when needed AND at the end of the loop
		//so we can use cv.wait
		while (true) {
			//i don't know why this is here, probably the lock
			//remains unlocked sometimes and needs to be locked
			//here?
			if (!lock.owns_lock()) lock.lock();

			//continue only if there is a command to execute OR
			//the printer is printing
			cv.wait(lock, [&] {return
				!global_printer->command_queue.empty()
				|| global_printer->state == Printing; });

			lock.unlock();

			//execute any commands that migth exist
			if (!global_printer->command_queue.empty()) {
				//using temporary variables to keep lock
				//unlocked

				lock.lock();
				PrinterCommands cmd =
					global_printer->command_queue.front();
				global_printer->command_queue.pop();
				lock.unlock();

				PrinterState state;
				if (cmd == PrinterCommands::Stop)
					state = PrinterState::Stoped;

				else if (cmd == PrinterCommands::Start
					|| cmd == PrinterCommands::Continue)
					state = PrinterState::Printing;

				else if (cmd == PrinterCommands::Pause)
					state = PrinterState::Paused;

				lock.lock();
				global_printer->state = state;
				lock.unlock();
			}

			if (global_printer->state == Printing) {
				//if line is empty, the last line was sent
				//succesfuly so we can read another

				if (line.length() == 0) {
					if (!std::getline(gcode_file, line)) {
						lock.lock();
						global_printer->state = Finished;

						//set global printer to NULL so
						//the gcode_thread does not
						//start sending the same file
						//again
						global_printer = NULL;

						gcode_file.close();

						//don't unlock the lock as the
						//outer loop expects in locked
						break;
					}
				}

				//if this is the last line, '\n' does
				//not end the line, so we need to put it
				//ourselfs
				line.append("\n");

				if (global_printer->send(line) < 0) {
					lock.lock();
					global_printer->state = Errored;
					lock.unlock();

					log("Printer errored while sending line!");

					//skip the rest because it errored
					//the lock remains unlocked
					continue;
				}
				else {
					lock.lock();
					lines_sent++;
					lock.unlock();

					//empty line so signal the line was
					//sent succesfuly
					line = "";
				}

				//search for the "ok" response
				do {
					if (global_printer->read_line(buffer) < 0) {
						lock.lock();
						global_printer->state = Errored;
						lock.unlock();

						log("Printer errored while reading line!");

						//break into the inner loop so
						//it waits until the error has
						//been cleared
						break;
					}
				} while(!global_printer->is_response_ok(buffer));
			}

		}
	}

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

	end_gcode_thread = true;
	std::thread gcode_thread(gcode_sender);
	log("Started gcode thread");

	client_loop();

	log("Ending gcode thread...");

	if (gcode_thread.joinable()) gcode_thread.join();
	log("Ended gcode thread");

	log("Exiting...\nThank you for using gcode_tui!");

	unlink(daemon_socket_addres.c_str());
	log_file.close();

	std::cout << "Hello world!" << std::endl;
	return 0;
}
