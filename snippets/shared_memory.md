# Shared Memory in C/C++

Shared memory is a mechanism that allows multiple processes to access the same region of memory, enabling fast inter-process communication (IPC). This snippet provides an overview and example of using POSIX shared memory in C.

## Overview

### Key Functions
1. **shm_open**: Creates or opens a shared memory object.
2. **ftruncate**: Sets the size of the shared memory object.
3. **mmap**: Maps the shared memory object into the process’s address space.
4. **munmap**: Unmaps the shared memory from the process’s address space.
5. **shm_unlink**: Removes the shared memory object.

### Include Headers
To use shared memory functions, include the following headers:
```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>
```

## Example: Writing to Shared Memory

Here is a simple program that creates a shared memory segment, writes a message to it, and cleans up resources:

```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define SHM_NAME "/my_shm"
#define SHM_SIZE 1024

int main() {
    // Create or open shared memory object
    int shm_fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Set size of shared memory object
    if (ftruncate(shm_fd, SHM_SIZE) == -1) {
        perror("ftruncate");
        return 1;
    }

    // Map shared memory to process's address space
    void *shm_ptr = mmap(0, SHM_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Write a message to shared memory
    const char *message = "Hello, Shared Memory!";
    strncpy((char *)shm_ptr, message, SHM_SIZE);

    printf("Message written to shared memory: %s\n", (char *)shm_ptr);

    // Cleanup
    munmap(shm_ptr, SHM_SIZE);
    shm_unlink(SHM_NAME);

    return 0;
}
```

## Example: Reading from Shared Memory

To read from shared memory, use the same shared memory name and map it into the process's address space:

```c
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <string.h>

#define SHM_NAME "/my_shm"
#define SHM_SIZE 1024

int main() {
    // Open existing shared memory object
    int shm_fd = shm_open(SHM_NAME, O_RDONLY, 0666);
    if (shm_fd == -1) {
        perror("shm_open");
        return 1;
    }

    // Map shared memory to process's address space
    void *shm_ptr = mmap(0, SHM_SIZE, PROT_READ, MAP_SHARED, shm_fd, 0);
    if (shm_ptr == MAP_FAILED) {
        perror("mmap");
        return 1;
    }

    // Read and print the message from shared memory
    printf("Message read from shared memory: %s\n", (char *)shm_ptr);

    // Cleanup
    munmap(shm_ptr, SHM_SIZE);

    return 0;
}
```

## Key Points
- Always clean up shared memory using `munmap` and `shm_unlink` to avoid resource leaks.
- Shared memory segments persist until explicitly unlinked with `shm_unlink`.
- Use synchronization mechanisms like semaphores or mutexes if multiple processes access shared memory concurrently.