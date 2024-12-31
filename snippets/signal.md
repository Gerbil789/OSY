# Signals in C/C++

**Signals** are asynchronous notifications sent to a process to notify it of events such as interrupts or exceptions. They are a key mechanism for interprocess communication in Unix-like systems.

- **Common Signals**:
  - `SIGINT`: Interrupt (e.g., Ctrl+C).
  - `SIGTERM`: Termination request.
  - `SIGKILL`: Kill process (cannot be caught or ignored).
  - `SIGSTOP`: Stop process (cannot be caught or ignored).
  - `SIGCONT`: Resume a stopped process.
  - `SIGSEGV`: Segmentation fault.

- **Key Functions**:
  - `signal(int signum, sighandler_t handler)`: Sets a handler for a signal.
  - `raise(int signum)`: Sends a signal to the calling process.
  - `kill(pid_t pid, int signum)`: Sends a signal to a specific process.
  - `sigaction(int signum, const struct sigaction *act, struct sigaction *oldact)`: Advanced signal handling.
  - `sigprocmask(int how, const sigset_t *set, sigset_t *oldset)`: Block/unblock signals.

- **Key Points**:
  - Handlers allow custom responses to signals.
  - `SIGKILL` and `SIGSTOP` cannot be caught, blocked, or ignored.
  - `sigaction` is preferred over `signal` for portability and advanced features.

**Example: Handling `SIGINT` with `signal`**:
```c
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>

void handle_signal(int sig) {
    printf("Caught signal %d. Exiting...\n", sig);
    exit(0);
}

int main() {
    signal(SIGINT, handle_signal); // Handle Ctrl+C
    while (1) {
        printf("Running... Press Ctrl+C to stop.\n");
        sleep(1);
    }
    return 0;
}
```

```c
#include <signal.h>
// Send a signal to the calling process
raise(SIGINT);

// Send a signal to another process
kill(pid, SIGTERM);
```