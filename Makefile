.PHONY: gcode_tui_daemon 

gcode_tui_daemon:
	g++ src/daemon/main.cpp src/commons.cpp\
		src/daemon/daemon_functions.cpp\
		src/daemon/StringCommand.cpp\
		src/daemon/Printer.cpp\
		-o gcode_tui_daemon -Wall
