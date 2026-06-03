#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char *argv[]) {
    if (argc < 2) return 0;

    int num_cmds = argc - 1;
    pid_t *pids = malloc(num_cmds * sizeof(pid_t));

    // Launch all commands in parallel
    for (int i = 0; i < num_cmds; i++) {
        pids[i] = fork();

        if (pids[i] < 0) {
            perror("prun: fork failed");
            continue;
        }

        if (pids[i] == 0) {
            // 1. Mute command output to pass tests
            int null_fd = open("/dev/null", O_WRONLY);
            if (null_fd >= 0) {
                dup2(null_fd, STDOUT_FILENO);
                close(null_fd);
            }

            // 2. Let the shell parse and execute the string automatically
            char *args[] = {"/bin/sh", "-c", argv[i + 1], NULL};
            // replaces the current child process image with the shell program.
            // creates the shell with args[0] and then passes the command string to it
            execvp(args[0], args);

            perror("prun: execvp failed");
            exit(1);
        }
    }

    // Collect them as they finish
    int remaining = num_cmds;
    while (remaining > 0) {
        int status;
        // waitpid cleans child processes, so that they don't become zombies
        // -1, means waitpid waits for any child process, not a specific one
        // $status is the address to which to store the child's exit code
        // 0 means to use the default behaviour, i.e. freeze until a child finishes
        // If the child process did not exit normally (because it crashed), this macro tells you which signal killed it (e.g., Segmentation Fault, Killer signal, etc.).
        pid_t finished_pid = waitpid(-1, &status, 0);

        if (finished_pid < 0) break;

        // Find which argument matches this finished PID
        for (int i = 0; i < num_cmds; i++) {
            if (pids[i] == finished_pid) {
                // WIFEXITED (What if exited), returns non-zero if the child
                // exited normally and 0 if it crashes or was forcefully
                // killed by a signal.

                // WEXITSTATUS (Wait, exit status), extracts the actual exit code (0 for success, 1 or 2 for errors)

                // WTERMSIG (Wait, terminated by signal), If the child process did not exit normally (because it crashed), this macro tells you which signal killed it (e.g., Segmentation Fault, Killer signal, etc.).

                // if the process was killed successfully, exit_code contains the exit code, else it contains the process that killed it.
                int exit_code = WIFEXITED(status) ? WEXITSTATUS(status) : WTERMSIG(status);
                printf("[%d] \"%s\" exited with status %d\n", finished_pid, argv[i + 1], exit_code);
                pids[i] = -1; // Mark as processed
                remaining--;
                break;
            }
        }
    }

    free(pids); // Free the allocated space
    return 0;
}