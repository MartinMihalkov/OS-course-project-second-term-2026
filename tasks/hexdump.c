#include <fcntl.h>  // File control, defines file manipultion and file opening control, and the access modes 
#include <unistd.h> // Unix standard, defines all sys calls
#include <string.h>
#include <stdio.h>

#define BUFFER_SIZE 16

// A funciton which writes a string to a given file descriptor by automatically finding the string's length.
void write_str(int fd, const char *str) {
    // write() requires that an address is given for it to read.
    write(fd, str, strlen(str)); // since str is already a pointer, we don't need to use '&'
}

int main(int argc, char *argv[]) {
    int fd;
    ssize_t bytes_read; // Will contain the number of bytes that have been read with read()
    unsigned char buf[BUFFER_SIZE];
    const char hex_chars[] = "0123456789ABCDEF"; // Used for the byte -> hex conversion
    // Checking whether there is exactly one argument passed. The first argument in argv[] is always the exec file path
    if (argc != 2) {
        write_str(2, "hexdump: Missing file argument\n"); // Writes the string to STDERR
        return 1;
    }

    fd = open(argv[1], O_RDONLY); // Opens the file using the 
    if (fd < 0) {
        perror("hexdump"); // Prints the error that occurred prefixed with "hexdump".
        return 1;
    }

    while ((bytes_read = read(fd, buf, BUFFER_SIZE)) > 0) { 
        // Loops while there is at least one byte left unread from the file
        int i;
        
        /* 1. Prints the hex representation. It cycles for the whole BUFFER_SIZE */
        for (i = 0; i < BUFFER_SIZE; i++) {
            if (i < bytes_read) {
                // If there are bytes which are not yet converted to hex, it converts them
                char hex[3];
                hex[0] = hex_chars[buf[i] >> 4];
                hex[1] = hex_chars[buf[i] & 0x0F];
                hex[2] = '\0';
                write_str(1, hex);
            } else {
                /* Else it fills missing bytes with spaces (2 spaces per byte) */
                write_str(1, "  ");
            }

            /* Handle spacing between bytes */
            if (i < BUFFER_SIZE - 1) { // We use "-1" so that no space is put after the last byte 
                // After the byte hex representation is printed, the spacing after it is decided
                if (i == 7) {
                    write_str(1, "  "); /* 2 spaces between 8th and 9th */
                } else {
                    write_str(1, " ");  /* 1 space otherwise */
                }
            }
        }

        /* 3 spaces to separate hex and ASCII */
        write_str(1, "   ");

        /* 2. ASCII representation loop */
        for (i = 0; i < BUFFER_SIZE; i++) { // Write all ASCII chars that will fit in BUFFER_SIZE
            if (i < bytes_read) { 
                // If there are still bytes left in the current BUFFER_SIZE, they should be written
                if (buf[i] >= 32 && buf[i] <= 127) {
                    // If the ASCII chars have a code between 32 and 127 (visible symbols), then print them.
                    write(1, &buf[i], 1); // Here we are giving the address of a char in the char array.
                } else {
                    // Else substitute them with "."
                    write_str(1, ".");
                }
            } else {
                // If there aren't characters anymore to print, pad the line with "."
                write_str(1, ".");
            }
        }

        write_str(1, "\n"); // At the end of each line a line feed is written.
    }

    close(fd); // Closing the file.
    return 0;
}