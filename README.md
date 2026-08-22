# gcode_tui

gcode_tui - drip gcode in the background to a machine

gcode_tui is a daemon(server) - client suite of (not the best written) programs
that controls and sends gcode to a machine.

**WARNING**: This project is still in it's infancy and even tough testing has 
accoured with a real 3D printer, it's not very thourough.

At the moment, only the daemon is finished and to control it you need to connect
to it with a program such as netcat.

## Features:
* Send gcode files in the background
* Get information about the print job
* pause/continue print

## Building
This project uses `cmake` to build

If you have not cloned the repo, clone it

```bash
git clone https://www.github.com/namenotfoundindb/gcode_tui
```

Go into the cloned folder if you have not already

```bash
cd gcode_tui
```

Then build the project

```bash
mkdir build
cd build
cmake ..
make
```

**DISCLAIMER**: At the moment you need to run this with root privleges because
gcode_tui_daemon needs to acces the `/run` (for the socket)  directory and
`/var/log` (for logging).

To run:

```bash
sudo ./gcode_tui_daemon
```

It might seem like nothing happened, but gcode_tui_daemon is running in the
background. Until i make a proper tui client, you can connect to it with netcat:

```bash
nc -N -U /run/gcode_tui_daemon.sock
```

Try typing the `help` command and see what other commands are there! For a more
indepth guide on using this, checkout USER_GUIDE.md.:w

## Future plans
* Add some sort of documentation (kinda important)
* Find a way to not have to run this as root
* Make an install script
* Automaticaly run this with `systemd`

## Thank you!
If you've reached the end of this readme, thank you for checking out my project!
