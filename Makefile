.PHONY: gcode_tui_daemon 

gcode_tui_daemon:
	g++ src/daemon/main.cpp -o gcode_tui_daemon -Wall
