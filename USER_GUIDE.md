# User guide
This user guide will take you through the basics of using gcode_tui_daemon.

If you are not already connected to the daemon:

```bash
nc -N -U /run/gcode_tui_daemon.sock
```

Now you should be connected, try sending the command `help`.

## Commands and arguments
gcode_tui_daemon has a different way of entering commands that what you normaly
think.

To enter a command, *first type the command you want to execute* (example:
`help`) and if the commands takes arguments, *type the argument **name**, then a
`':'` followed by the argument **value***. If the argument contains ***spaces**,
put the argument value in **quotes***.

### General command scheme
```
<command> <argument name>:<argument value> ...
```

### Examples

If an argument has *no spaces*:

```
echo text:Hello
```

If an argument *has spaces*:

```
echo text:"Hello world!"
```

### Notes:
* Arguments and their values can not contain colons
* The is no problem if you saround an argument value with no spaces in quotes

## The Printer
This part will explain how to take care of the printer

### Initializing
Before doing anything with the printer it must be initilized with the `'init'`
command. The init command takes as arguments the `printer` witch is the location
of the *printers character device file* (usualy in `/dev`) and `baudrate` witch
is the baudrate. Here's an example:

```
init printer:/dev/ttyUSB0 baudrate:9600
```

### Sending a file
Now that the printer is initilized, you can send a gcode file with the `send`
command. It takes one argument: `file`, the gcode file to send. Example:

```
send file:/home/user/cube.gcode
```

If it cannot open the file, it will tell you.

### Getting status
The `status` command gives printer status. Just one caveat, the `state` field
is not in words like idle, printing, errored its a number because it was
designed to be interpreted by the client.

#### Printer states:
0) Uninitialized
1) Errored
2) Printing
3) Idle
4) Stoped
5) Paused
6) Finished

If you know some tehnical stuff, the enum that store these constants is in
`src/daemon/Printer/State.hpp`.

**NOTE:** In some printer states, it may not print all the statistics.

## General daemon commands
* `exit` - disconnect from the daemon
* `shutdown` - shutdown the daemon (there is no warning if the printer is
  printing)

# Logs
gcode_tui_daemon's log file is `/var/log/gcode_tui_daemon.log`.

**IMPORTANT**: It gets overwritten every time gcode_tui_daemon restarts! Gotto
work on that.
