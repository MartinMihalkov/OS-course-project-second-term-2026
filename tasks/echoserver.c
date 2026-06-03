#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

// Signal handler to clean up finished child processes (reaps zombies)
void handle_sigchld(int sig) {
    // WNOHANG makes waitpid non-blocking so the main loop never stutters
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void handle_client(int client_fd) {
    char buf[1024];
    ssize_t n;

    // Read blocks of 1024 bytes and echo them back verbatim
    while ((n = read(client_fd, buf, sizeof(buf))) > 0) {
        write(client_fd, buf, n);
    }

    close(client_fd);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "echoserver: missing port\n");
        return 1;
    }

    // Register the SIGCHLD handler
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART; // Prevents interrupted system calls (like accept) from crashing
    if (sigaction(SIGCHLD, &sa, NULL) < 0) {
        perror("echoserver: sigaction");
        return 1;
    }

    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("echoserver");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {0};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(atoi(argv[1]));

    if (bind(server_fd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
        perror("echoserver");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 10) < 0) {
        perror("echoserver");
        close(server_fd);
        return 1;
    }

    // Infinite server loop
    while (1) {
        int client_fd = accept(server_fd, NULL, NULL);
        if (client_fd < 0) {
            // accept might fail if interrupted by our SIGCHLD signal handler
            continue; 
        }

        pid_t pid = fork();
        if (pid == 0) {
            // --- CHILD PROCESS ---
            close(server_fd); // Child doesn't need the listener socket
            handle_client(client_fd);
            exit(0);          // Exit child cleanly when communication finishes
        } else if (pid > 0) {
            // --- PARENT PROCESS ---
            close(client_fd); // Parent doesn't need this specific client descriptor anymore
        } else {
            perror("echoserver: fork failed");
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}