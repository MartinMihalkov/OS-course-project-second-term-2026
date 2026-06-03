#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int main(int argc, char *argv[]) {
    if (argc != 4) {
        fprintf(stderr, "echoclient: missing arguments\n");
        return 1;
    }

    char *hostname = argv[1];
    int port = atoi(argv[2]);
    char *message = argv[3];

    struct hostent *server = gethostbyname(hostname);
    if (!server) {
        fprintf(stderr, "echoclient: host not found\n");
        return 1;
    }

    int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        perror("echoclient");
        return 1;
    }

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    memcpy(&addr.sin_addr.s_addr, server->h_addr_list[0], server->h_length);
    addr.sin_port = htons(port);

    if (connect(sock_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("echoclient");
        close(sock_fd);
        return 1;
    }

    // Send the message string directly
    write(sock_fd, message, strlen(message));

    // Read back the echo response
    char buf[1024];
    ssize_t n = read(sock_fd, buf, sizeof(buf) - 1);
    if (n > 0) {
        buf[n] = '\0';
        printf("%s", buf);
    }

    close(sock_fd);
    return 0;
}