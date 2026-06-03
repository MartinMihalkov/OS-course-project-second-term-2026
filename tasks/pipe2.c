#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h> // wait() and waitpid(), WIFEXITED, WEXITSTATUS, WTERMSIG
#include <sys/types.h> // pid_t and others

int main(int argc, char *argv[]) {
    // Check that exactly two arguments are passed
    if (argc != 3) {
        write(2, "pipe2: Invalid arguments\n", 25);
        return 1;
    }

    // getting the two arguments
    char *cmd1 = argv[1];
    char *cmd2 = argv[2];

    int pipefd[2];
    // pipe() creates an anonymous pipeline in RAM and gives two new file descriptors.
    // you read pipefd[0]'s fd and write to pipefd[1]
    // Pipes will block if empty: If a process tries to read() from a pipe but the writer hasn't sent anything yet, the reader will freeze (block) and sleep until data arrives.

    // Closing is mandatory: If the writer process finishes but forgets to close its copy of pipefd[1], the reader process will hang forever waiting for more data, never realizing that the writer is done.

    if (pipe(pipefd) < 0) {
        perror("pipe2: pipe failed");
        return 1;
    }

    // Fork the first child (Left Command: cmd1)
    pid_t pid1 = fork();
    if (pid1 < 0) {
        perror("pipe2: fork 1 failed");
        return 1;
    }

    if (pid1 == 0) {
        // Inside Child 1: Redirect stdout to the write end of the pipe
        // duplicates write end to the standard OUTPUT!!!!!
        // It says: close this child's connection to the terminal screen, and redirect its standard output into the write-end of our pipe instead.
        dup2(pipefd[1], STDOUT_FILENO);
        
        // Close the two pipe ends
        close(pipefd[0]);
        close(pipefd[1]);

        // Execute using shell execution to automatically handle string spaces
        // NULL marks the absolute end of the array. execvp relies on this NULL marker to know when to stop reading arguments.
        char *args[] = {"/bin/sh", "-c", cmd1, NULL};
        // replacese the current running program with a brand-new program.
        execvp(args[0], args);
        
        perror("pipe2: execvp 1 failed");
        exit(1);
    }

    // Fork the second child (Right Command: cmd2)
    pid_t pid2 = fork();
    if (pid2 < 0) {
        perror("pipe2: fork 2 failed");
        return 1;
    }

    // Only execute the following code if the program is running inside the child process, so that this code
    // doesn't run in the parent process.
    //In the Child process: fork() returns exactly 0.
    // In the Parent process: fork() returns a positive number (the actual Process ID of the new child).
    if (pid2 == 0) {
        // Inside Child 2: Redirect stdin to the read end of the pipe
        // Closing pipefd[0] afterwards just cleans up the temporary variable; the connection to the pipe remains open via STDIN_FILENO.
        // Redirects the read end to the standard INPUT!!!!
        // Redirect standard INPUT so it reads from the pipe.
        dup2(pipefd[0], STDIN_FILENO);

        // Before Child two closes the pipe ends it duplicates the read end onto standard input.
        // closing the two pipe ends
        close(pipefd[0]);
        close(pipefd[1]);

        // Execute using shell execution
        char *args[] = {"/bin/sh", "-c", cmd2, NULL};
        execvp(args[0], args);
        
        perror("pipe2: execvp 2 failed");
        exit(1);
    }

    // Closing the two pipe ends
    close(pipefd[0]);
    close(pipefd[1]);

    int status1, status2;
    
    // Explicitly wait for both children by their specific PIDs
    waitpid(pid1, &status1, 0);
    waitpid(pid2, &status2, 0);

    // Calculate exit code based on the rightmost command (cmd2)
    int exit_code = 0;
    if (WIFEXITED(status2)) {
        exit_code = WEXITSTATUS(status2);
    } else if (WIFSIGNALED(status2)) {
        // we use 128 + signal, so the shell can distinguish signal termination from ordinary exit values.
        exit_code = 128 + WTERMSIG(status2);
    }

    return exit_code;
}