# Polling in C/C++

**Polling** is a mechanism to monitor multiple file descriptors (e.g., pipes, sockets) for events such as readability, writability, or errors.

- **Function**:
  - `int poll(struct pollfd *fds, nfds_t nfds, int timeout)`
    - `fds`: Array of `struct pollfd` describing the file descriptors to monitor.
    - `nfds`: Number of file descriptors in the array.
    - `timeout`: Timeout in milliseconds (`-1` for infinite, `0` for non-blocking).

- **Returns**:
  - Positive: Number of file descriptors with events.
  - `0`: Timeout occurred.
  - `-1`: Error.

- **`struct pollfd`**:
  ```c
  struct pollfd {
      int fd;       // File descriptor to monitor
      short events; // Events to watch (e.g., POLLIN, POLLOUT)
      short revents;// Events that occurred
  };
	```

- **Key Points:**
  - Used for efficient I/O multiplexing.
  - Can replace `select()` for monitoring many file descriptors.

**example:**

```c
#include <poll.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    int pipefd[2];
    pipe(pipefd);
    const char *message = "Polling Example";
    struct pollfd fds[1];

    if (fork() == 0) {
        // Child process
        close(pipefd[0]); // Close unused read end
        sleep(1);         // Simulate delay
        write(pipefd[1], message, sizeof(message));
        close(pipefd[1]);
    } else {
        // Parent process
        close(pipefd[1]); // Close unused write end
        fds[0].fd = pipefd[0];
        fds[0].events = POLLIN;

        int ret = poll(fds, 1, 5000); // Wait up to 5 seconds
        if (ret > 0 && (fds[0].revents & POLLIN)) {
            char buffer[20];
            read(pipefd[0], buffer, sizeof(buffer));
            printf("Parent received: %s\n", buffer);
        } else if (ret == 0) {
            printf("Timeout occurred\n");
        } else {
            perror("poll failed");
        }
        close(pipefd[0]);
    }
    return 0;
}
```