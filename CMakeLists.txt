cmake_minimum_required(VERSION 3.15)

project(
	gcode_tui_daemon
	LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(gcode_tui_daemon
	src/daemon/main.cpp
	src/commons.cpp
	src/daemon/daemon_functions.cpp
	src/daemon/int_to_termios_baudrate.cpp
	src/daemon/Printer/Printer.cpp
	src/StringCommand.cpp
	src/string_functions.cpp
	src/Cson.cpp
)
