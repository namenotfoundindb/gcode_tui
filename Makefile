.PHONY: gcode_tui_daemon 

gcode_tui_daemon:
	g++ src/daemon/main.cpp src/commons.cpp\
		src/daemon/daemon_functions.cpp\
		src/daemon/StringCommand.cpp\
		src/daemon/Printer/Printer.cpp\
		src/daemon/string_functions.cpp\
		-o gcode_tui_daemon -Wall -std=c++20

debug:
	g++ src/daemon/main.cpp src/commons.cpp\
		src/daemon/daemon_functions.cpp\
		src/daemon/StringCommand.cpp\
		src/daemon/Printer/Printer.cpp\
		src/string_functions.cpp\
		-o gcode_tui_daemon -Wall --debug

