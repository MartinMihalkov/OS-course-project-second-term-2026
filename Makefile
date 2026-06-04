# Defines the compiler and its flags
CC = gcc
CFLAGS = -Wall -Wextra -O2 # Includes warnings, important for POSIX calls.

# List with all targets
TARGETS = hexdump reverse prun pipe2 worker timeserver timeclient echoserver echoclient

# Main target - compiles everything on execution of "make"
all: $(TARGETS)

# Tasks are located in the tasks/ directory, but compiled into root
hexdump: tasks/hexdump.c
	$(CC) $(CFLAGS) tasks/hexdump.c -o hexdump

reverse: tasks/reverse.c
	$(CC) $(CFLAGS) tasks/reverse.c -o reverse

prun: tasks/prun.c
	$(CC) $(CFLAGS) tasks/prun.c -o prun

pipe2: tasks/pipe2.c
	$(CC) $(CFLAGS) tasks/pipe2.c -o pipe2

worker: tasks/worker.c
	$(CC) $(CFLAGS) tasks/worker.c -o worker

timeserver: tasks/timeserver.c
	$(CC) $(CFLAGS) tasks/timeserver.c -o timeserver

timeclient: tasks/timeclient.c
	$(CC) $(CFLAGS) tasks/timeclient.c -o timeclient

echoserver: tasks/echoserver.c
	$(CC) $(CFLAGS) tasks/echoserver.c -o echoserver

echoclient: tasks/echoclient.c
	$(CC) $(CFLAGS) tasks/echoclient.c -o echoclient

# Clearing of all executable files
clean:
	rm -f $(TARGETS)

.PHONY: all clean