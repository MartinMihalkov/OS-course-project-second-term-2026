#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define BUFFER_SIZE 512

int main(int argc, char *argv[]) {
    if (argc != 2) {
        write(2, "reverse: Invalid arguments\n", 27);
        return 1;
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("reverse");
        return 1;
    }

    // lseek changes the current position of the read/write offset
    // returns the new positoin in the file
    // SEEK_END the end of the file
    // SEEK_SET the start of the file
    // SEEK_CUR the current position of the offset.


    // lseek(fd, (can be negative) number of bytes to move the pointer, relative to the third parameter)
    // (0 for SEEK_SET, 1 for SEEK_CUR and 2 for SEEK_END)

    // if you use offset that surpases the end of file, lseek will just
    // move the pointer into empty space.
    // if you read(), it will give 0, i.e. it has reached EOL
    // if you write(), it will fill the gap between the current position and
    // previous last intex with /0.

    // gets the length of the file
    // off_t is used for file sizes. It can automatically scale behind the scenes.
    off_t file_size = lseek(fd, 0, SEEK_END);
    if (file_size < 0) {
        perror("reverse");
        close(fd);
        return 1;
    }

    // A single buffer is all we need
    char buf[BUFFER_SIZE]; 
    off_t left = 0; // the first byte index
    off_t right = file_size - 1; // the last byte index

    // Loop until the two pinter don't meet each other in the middle
    while (left < right) {
        // contains the left and right char to be swapped
        char left_char, right_char; 

        // Moves the pointer to the left byte and reads it
        lseek(fd, left, SEEK_SET);
        read(fd, &left_char, 1);

        // Moves the pointer to the right byte and reads it
        lseek(fd, right, SEEK_SET);
        read(fd, &right_char, 1);

        // Swap and write them back to opposite locations
        lseek(fd, left, SEEK_SET);
        write(fd, &right_char, 1);

        lseek(fd, right, SEEK_SET);
        write(fd, &left_char, 1);

        // Move markers inward
        left++;
        right--;
    }

    close(fd); // Close the file
    return 0;
}