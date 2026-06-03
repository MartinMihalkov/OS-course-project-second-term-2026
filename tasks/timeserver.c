#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "timeserver: missing port\n");
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("timeserver");
        return 1;
    }

    int opt = 1;


    // When a TCP connection closes, the operating system kernel doesn't release the network port instantly. It forces the port into a protective state called TIME_WAIT for 1 to 2 minutes.

    // This is a built-in safety feature of TCP to catch any late or delayed data packets still floating around the internet from the old connection, preventing them from accidentally corrupting a brand-new process.

    // This line tells the server to immediately release the port,
    // when the server process closes.
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Creates addr structure for IPv4 data. Wipes any garbage data in RAM.
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    // Which NIC the server should listen on.
    addr.sin_addr.s_addr = INADDR_ANY; // 0.0.0.0 (listen from all NICs)
    // Big engian (stores the MSB (not bit) first)
    // Little endian (stores the LSB (not bit) fist)
    // Most protocols use BE.
    // Most CPUs use LE.
    // htons flips the bytes.
    // atoi() gets the port number and converts it to int.
    addr.sin_port = htons(atoi(argv[1]));

    // binds a socker which is only created in a namespace, with an IP address.
    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("timeserver");
        close(server_fd);
        return 1;
    }

    // 1 means that at most 1 connection to the socket can be initiated at a time
    // assigns the socket to be listening
    listen(server_fd, 1);

    // Creates a socket based on the first request to the server socket.
    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd < 0) {
        perror("timeserver");
        close(server_fd);
        return 1;
    }

    // Get and format time
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S\n", tm_info);

    write(client_fd, buf, strlen(buf));

    close(client_fd);
    close(server_fd);
    return 0;
}