# Technical documentation

This document explains the inner workings of gcode_tui.

**NOTE:** I think this document covers the most important parts of how this
program works, *but it is not finished*. If there is a *difference* to what this
says and what the code says, the *code wins*.

## The Printer class
It is a way of accesing the printer without writting directly to file
descriptors. First it must be initilized with the *termios baudrate* and path to
the *character device filename* using the `init` method.

After it is used it can be disconnected with `disconnect`.

### int_to_termios
This is a `std::map` in a seperate file that is used to convert from a baudrate
stored in an integer to a baudrate that `termios` will understand.

### Reading from the printer
Reading from a Printer is done using `Printer::read_line` and
`Printer::read_char`, which do what you think the do.

`read_line` uses `read_char` and reads until a newline.

`read_char` tries to read until it acctualy read something (it checks this using
`errno`, if `errno` == EAGAIN then there was no data to read) if it could not read
it waits a bit and tries again. It will return -1 if errno != EAGAIN.

## Multithreaded design

gcode_tui_daemon runs on two threads:

* client thread (main thread) - listens to client commands and holds the client.
  It mainly is in the `client_loop` function.
* `gcode_thread` - talks to the printer, and sends gcode, ablivious to what the
  client is sending over. It executes the `gcode_sender` function.

### `global_printer`
It is a pointer pointing to a `Printer` that should be considered the main
printer. If this is NULL, the `gcode_thread` is instructed to wait until it is
not NULL. In a way, this is the message to the `gcode_thread` that the printer
is connected and initialized.

### How it sends gcode lines
It firsts reads a gcode file from the `gcode_file`, if it can't it assumes it
finished the gcode_file. Next it sends that line and starts reading from the
printer, waiting for a `ok` to appear in the first 2 bytes of the response
(that's how it know that it can send the next line). The cycle repeats until the
end of the file.


### gcode_thread
It acceses shared resources (like `global_printer` through a the `std::mutex`
`mtx`. Off of this mutex, an `std::unique_lock` named `lock` is created inside
the `gcode_sender` function that is used to acces shared resources.

Using `lock` and `cv` ( a global `std::condition_variable`) it waits until
`global_printer` is not NULL (thus printer is initialized) and enters the inner
loop.

In the inner loop `cv` waits again until there are some printer commands in the
queue (`!global_printer->command_queue.empty()`) OR the printer is printing
(`global_printer->state == Printing`) OR the gcode thread should end
(`end_gcode_thread == true`).

After it continues and checks if it should *exit*, it *interprets the Printer
commands* in the queue and *sends gcode*. If it *can't read* from the
`gcode_file`, it assumes it finished reading it and *finishes printing* (sets
the printer state to Finished, sets `global_printer` to NULL, and exits the
inner loop).

## client - daemon communication

To communicate with the daemon, the client connects to the **unix socket**
(currently located `/run/gcode_tui_daemon.sock`, if not check `src/commons.cpp`,
it should contain some configuration options).

### UserCommand
`UserCommand` is a *formatting protocol* designed to procces commands and
arguments between the client and the daemon. The format syntax works like this:

#### Syntax

```
<command> <argument 1 name>:<argument 1 value> ...
```

* `command` - the command to execute
* `argument 1 name` - the name of argument one
* `:` - a colon to separate the argument from the value
* `argument 1 value` - the value of argument 1, **if it contains spaces it
  should be sourounded in double quotes**.

Argument - key *pairs may continue for a teortical infinity* (and are practicaly
limited by the maximum number of elements in a `std::map`)

#### UserCommand class
The UserCommand class is a class that helps with proccesing a UserCommand.

I am not going the enumerate each field and method in this class, but just know
the following notes:

* Idealy arguments should be **accesed using the `get_arg`** method.
* To *populate the fields* from a UserCommand string, use
  *`UserCommand::parse`*.

### Cson
Cson is data storage format *very similar to UserCommand*, but with just no
`command` and arguments are **separated using newlines and not spaces**.

#### Cson class
It has the same `parse` method as UserCommand and should be used before trying
to acces any data.

To get a value from it use `get(std::string key, std::string* dest)`. This
function returns a boolean that tells you if the requested key was found. If the
value has found it gets put into the string pointed to by `dest` (yes i know
this is a very c way of doing it).

## Command system
This system is used to maintain an organized codebase, instead of a switch
statement that decides what each command does.

### CommandContext struct
This is a structure containg *information from where the command is executed*,
otherwise know as **context**. It contains the UserCommand proccesed with the
arguments, a printer, the `global_printer` passed by reference, the clients file
desctiptor and so on.

It basicaly exists so that a commands action can impact what the program does
outside of it's scope.

### Command struct
The `Command` structure *holds information about commands* (excluding their name)
like a `desctiption`, some `required_arguments` and a function to execute when
called (their `action`).

Idealy the `action` should be a function in the `Commands::Functions` namespace.
These functions should be declared in `src/daemon/Commands.hpp` and defined in
`src/daemon/Commands.cpp`, alongside the respective `Command` struct.

`Command` structs should be in `src/daemon/Commands.cpp` and externed in
`src/daemon/Commands.hpp`.

Eearlier i said that the `Command` struct *does not contain a command name*.
Why? Because each *`Command` struct is associated in an `std::map` with a
`std::string`* so that it can search through the map for the command name, and
get a `Command` in return. In fact there is a function that finds a command by
name from the `Commands::list` map and executes it.

It is called `find_and_execute_Command` and it takes as argument a
`CommandContext`. If it found a command with the name from the context
`UserCommand.command` , then it calls `try_execute_Command` wich uses the
arguments from `context.usrcmd.arguments` and the commands required arguments to
check if there are enough arguments, the it executes the commands action,
passing the context along.

**Quick summary of the last paragraph:**
I know i didn't write the last part in an understandable way, but basicaly,
`find_and_execute_Command` finds the command and then calls
`try_execute_Command` wich executes it if enough arguments are given.

### The nice part
The beauty of this system is that *it's easier* to add new commands, for
example, when you add a new command in `Commands::list`, when the `help` command
gets executed it will just iterate through `Commands::list` and search through
command desctiptions. In the future it could also print their
`required_arguments`.

I could also make `Commands::list` not be constant so you can add commands on
the fly, as the program is executing. Maybe i should try adding an extensions
system.

## How commands are stored and interpretted
When a user sends a command, in the `client_loop` function, the `read` system
call returns the command string.

That command string is loaded into an `std::string` wich itself gets proccesed
into a UserCommand object.

Then a `CommandContext` struct is filled up with the context and it gets sent to
`find_and_execute_Command`.

If the command is found, it will execute it with `try_execute_Command`.

If enough arguments are given, it will call `command.action` passing along the
`CommandContext`.
