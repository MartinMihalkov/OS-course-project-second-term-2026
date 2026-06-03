#include <stdio.h>
#include <unistd.h>
#include <signal.h>

// SIGKILL - kills the process immediately
// SIGINT and SIGTERM don't force the program
// - SIGINT (SIGNAL INTERRUPT) - SENT BY Ctrl+C in the terminal where the pr. is running
// - SIGTERM (SIGNAL TERMINATE) - SEND BY ANOTHER PROCESSS or the OS, or the kill command

// volatile tells the compiler not to optimize the variable
// if modern compilers see that a var. is not used withing a loop, they store
// it in really fast register instead of checking it from RAM 
// Global flags strictly using the required type
// Else the compiler will replace while(var), with while(1) if it sees var never changes.

//sig_atomic_t tells C, that the var, can be read, or written in a single "atomic" operation, a single uninterupted state.
volatile sig_atomic_t keep_running = 1;
// It flips to 1 the moment SIGUSR1 hits, signaling the main loop to print a status update.
volatile sig_atomic_t usr1_received = 0;

// Signal handler for SIGINT and SIGTERM
void handle_term(int signum) {
    keep_running = 0;
}

// Signal handler for SIGUSR1
void handle_usr1(int signum) {
    usr1_received = 1;
}

int main() {
    // Line buffer stdout so text prints immediately even when run in background
    setvbuf(stdout, NULL, _IOLBF, 0);

    // Map the handlers to a specific action
    struct sigaction sa_term;
    sa_term.sa_handler = handle_term;
    // initializes a sigset_t so it contains no signals. there is also sigfillset, sigaddset and sigdelset to modify the set.
    sigemptyset(&sa_term.sa_mask);
    sa_term.sa_flags = 0;

    // Register both shutdown signals
    // installs signal handlers
    // sa_handler: the function runs when the signal arrives
    sigaction(SIGINT, &sa_term, NULL);
    sigaction(SIGTERM, &sa_term, NULL);

    struct sigaction sa_usr1;
    sa_usr1.sa_handler = handle_usr1;
    // sa_mask: sigset_t of signals to block while the handler runs
    sigemptyset(&sa_usr1.sa_mask);
    // behavior modifiers ()
    sa_usr1.sa_flags = 0;

    // Register the status signal
    sigaction(SIGUSR1, &sa_usr1, NULL);

    int counter = 0;

    while (keep_running) {
        // sleep(1) can be interrupted early by a signal.
        // We only tick and print if sleep completes or if we haven't checked yet.
        sleep(1);

        // Check if we need to exit immediately before doing anything else
        if (!keep_running) {
            break;
        }

        counter++;
        printf("tick %d\n", counter);

        // Check if SIGUSR1 was caught during this second
        if (usr1_received) {
            printf("status: tick %d\n", counter);
            usr1_received = 0; // Reset flag
        }
    }

    // Graceful shutdown message using the last valid counter value
    printf("shutdown after %d ticks\n", counter);

    return 0;
}