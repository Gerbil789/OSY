# `fork()` in C/C++

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

# `wait()` and `waitpid()` in C/C++

`wait()` and `waitpid()` are system calls used by a parent process to wait for its child processes to terminate.

## `wait()`
- **Function**:
  ```c
  #include <sys/types.h>
  #include <sys/wait.h>
  pid_t wait(int *status);
  ```
- **Description**:
  - Suspends the execution of the calling process until one of its child processes terminates.
  - Returns the PID of the terminated child.
  - The `status` argument is used to return information about how the child process terminated.

- **Key Points**:
  - If there are no child processes, `wait()` returns `-1`.
  - If a child process has already terminated, `wait()` returns immediately.

**Example**:
```c
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        printf("Child process: PID = %d\n", getpid());
    } else if (pid > 0) {
        // Parent process
        int status;
        wait(&status);
        printf("Parent process: Child terminated.\n");
    } else {
        // Error
        perror("fork failed");
    }
    return 0;
}
```

## `waitpid()`
- **Function**:
  ```c
  #include <sys/types.h>
  #include <sys/wait.h>
  pid_t waitpid(pid_t pid, int *status, int options);
  ```
- **Description**:
  - Provides more control over which child process to wait for.
  - Can specify a particular child process or use macros like `-1` (wait for any child process).
  - The `options` argument allows non-blocking or other behaviors.

- **Key Points**:
  - If `pid > 0`, waits for the child process with the specified PID.
  - If `pid == 0`, waits for any child process in the same process group.
  - If `pid == -1`, waits for any child process.
  - If `pid < -1`, waits for any child process in the process group `|pid|`.

**Example**:
```c
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        printf("Child process: PID = %d\n", getpid());
    } else if (pid > 0) {
        // Parent process
        int status;
        waitpid(pid, &status, 0);
        printf("Parent process: Child %d terminated.\n", pid);
    } else {
        // Error
        perror("fork failed");
    }
    return 0;
}
```

