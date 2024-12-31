# `dup2` in C/C++

The `dup2` system call is used in Unix-like operating systems to duplicate a file descriptor. It allows you to redirect one file descriptor to another, making it useful for tasks like redirection of standard input, output, or error streams.

## Syntax

```c
#include <unistd.h>

int dup2(int oldfd, int newfd);
```

- `oldfd`: The file descriptor you want to duplicate.
- `newfd`: The file descriptor you want `oldfd` to be duplicated to. If `newfd` is already open, it will be closed before duplication.

### Return Value
- On success: Returns `newfd`.
- On failure: Returns `-1` and sets `errno`.

## Description
The `dup2` system call duplicates `oldfd` to `newfd`. After duplication, both file descriptors refer to the same underlying file or socket, and they share the same file offset and status flags.

If `newfd` is the same as `oldfd`, `dup2` does nothing and simply returns `newfd`.

## Example
The following example demonstrates using `dup2` to redirect the standard output to a file:

```c
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>

int main() {
    int file = open("output.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (file < 0) {
        perror("open");
        exit(EXIT_FAILURE);
    }

    // Redirect standard output to the file
    if (dup2(file, STDOUT_FILENO) < 0) {
        perror("dup2");
        close(file);
        exit(EXIT_FAILURE);
    }

    // Now, all output to stdout will go to the file
    printf("This will be written to the file\n");

    // Close the file descriptor
    close(file);

    return 0;
}
```

## Explanation
1. **Open the file**: The `open` system call opens the file `output.txt` for writing, creating it if it does not exist.
2. **Redirect standard output**: The `dup2` system call redirects `STDOUT_FILENO` (file descriptor 1) to the file descriptor returned by `open`.
3. **Write to the file**: After redirection, any output to `stdout` (e.g., `printf`) is written to the file instead of the terminal.
4. **Close the file**: The file descriptor is closed to free system resources.

## Notes
- Always check the return value of `dup2` for errors.
- Ensure that `oldfd` is valid before calling `dup2`.
- The `dup2` system call is a powerful tool for implementing I/O redirection in applications.
