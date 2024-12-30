

### `fork()` in C/C++

`fork()` is a system call in Unix-like operating systems that creates a new process by duplicating the calling process. The new process is called the child process, and the original is the parent process.

- **Return Values**:
  - `0` in the child process.
  - PID of the child process in the parent.
  - `-1` on failure.

- **Key Points**:
  - Both processes start execution right after the `fork()` call.
  - They share the same code but have separate memory spaces.
  - Commonly used in process creation for multitasking.

**Example**:
```c
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        printf("Child process: PID = %d\n", getpid());
    } else if (pid > 0) {
        // Parent process
        printf("Parent process: PID = %d, Child PID = %d\n", getpid(), pid);
    } else {
        // Error
        perror("fork failed");
    }
    return 0;
}
```