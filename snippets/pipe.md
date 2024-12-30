### Pipes in C/C++

A **pipe** is a unidirectional communication mechanism between processes. It allows one process to write data, while another reads it, typically used between parent and child processes.

- **Functions**:
  - `pipe(int pipefd[2])`: Creates a pipe.
    - `pipefd[0]`: Read end.
    - `pipefd[1]`: Write end.
  - Returns `0` on success, `-1` on failure.

- **Key Points**:
  - Data written to `pipefd[1]` can be read from `pipefd[0]`.
  - Requires a parent-child relationship or shared ancestry.
  - Pipes do not store data indefinitely; reading removes it.

**Example**:
```c
#include <unistd.h>
#include <stdio.h>

int main() {
    int pipefd[2];
    char buffer[20];
    const char *message = "Hello, Pipe!";

    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        return 1;
    }

    if (fork() == 0) {
        // Child process
        close(pipefd[1]); // Close unused write end
        read(pipefd[0], buffer, sizeof(buffer));
        printf("Child received: %s\n", buffer);
        close(pipefd[0]);
    } else {
        // Parent process
        close(pipefd[0]); // Close unused read end
        write(pipefd[1], message, sizeof(message));
        close(pipefd[1]);
    }
    return 0;
}
```