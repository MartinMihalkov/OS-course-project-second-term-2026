#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "timeclient: missing arguments\n");
        return 1;
    }

    // Makes DNS query
    struct hostent *server = gethostbyname(argv[1]);
    if (!server) {
        fprintf(stderr, "timeclient: host not found\n");
        return 1;
    }

    // creates a socket
    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("timeclient");
        return 1;
    }

    // creates a struct
    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    addr.sin_port = htons(atoi(argv[2]));

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("timeclient"); // Outputs "timeclient: Connection refused" natively on failure
        close(sock_fd);
        return 1;
    }

    char buf[128];
    ssize_t n;
    while ((n = read(sock_fd, buf, sizeof(buf) - 1)) > 0) {
        buf[n] = '\0';
        printf("%s", buf);
    }

    close(sock_fd);
    return 0;
}