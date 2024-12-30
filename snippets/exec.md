
### `exec` in C/C++

The `exec` family of functions in Unix-like operating systems replaces the current process image with a new one. It is used to run a different program within the same process.

#### Key Functions
- **Common Variants**:
  - `execl`: Takes a list of arguments.
  - `execv`: Takes an array of arguments.
  - `execle`: Takes a list of arguments and an environment array.
  - `execve`: Takes an array of arguments and an environment array.
  - `execlp`: Similar to `execl`, searches for the file in `PATH`.
  - `execvp`: Similar to `execv`, searches for the file in `PATH`.

- **General Syntax**:
```c
#include <unistd.h>

int execl(const char *path, const char *arg, ..., NULL);
int execv(const char *path, char *const argv[]);
int execle(const char *path, const char *arg, ..., NULL, char *const envp[]);
int execve(const char *path, char *const argv[], char *const envp[]);
int execlp(const char *file, const char *arg, ..., NULL);
int execvp(const char *file, char *const argv[]);
```

- **Key Points**:
- Replaces the current process image; does not return on success.
- The process retains the same PID.
- On failure, it returns `-1` and sets `errno`.


- **Example: `execl`**:
```c
#include <unistd.h>
#include <stdio.h>

int main() {
    printf("Executing `ls` command...\n");
    execl("/bin/ls", "ls", "-l", NULL);
    perror("execl failed"); // This will only execute if execl fails
    return 1;
}
```

- **Example: `execvp`**:
```c
#include <unistd.h>
#include <stdio.h>

int main() {
    char *args[] = {"ls", "-l", NULL};
    printf("Executing `ls` command...\n");
    execvp("ls", args);
    perror("execvp failed"); // This will only execute if execvp fails
    return 1;
}
```

- **Example: `fork` and `exec`**:
```c
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

int main() {
    pid_t pid = fork();

    if (pid == 0) {
        // Child process
        char *args[] = {"ls", "-l", NULL};
        execvp("ls", args);
        perror("execvp failed");
    } else if (pid > 0) {
        // Parent process
        wait(NULL); // Wait for the child process to finish
        printf("Child process finished.\n");
    } else {
        perror("fork failed");
    }
    return 0;
}
```

- **Key Points**:
- When to Use:
  - To replace the current process with a new program.
  - Commonly used in conjunction with fork to spawn a new process and execute a different program in the child process.
- Important Notes:
  - Ensure the target program exists and is executable.
  - Always handle errors (e.g., missing program, permission issues).
  - Arguments must end with NULL to signal the end of the list.