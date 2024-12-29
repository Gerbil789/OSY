# Operační systémy - VSB
https://poli.cs.vsb.cz/edu/osy

✅ **Done:** `zadani01`, `zadani02`, `zadani03`, `zadani05`

❌ **Todo:** `zadani04`, `zadani06`, `zadani07`, `zadani08`, `zadani09`, `zadani10`, `zadani11`

---

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
---
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

---
### Polling in C/C++

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

---
### Sockets in C/C++

A **socket** is an endpoint for communication between processes over a network. It can be used for both connection-oriented (TCP) and connectionless (UDP) communication.

- **Socket Types**:
  - `SOCK_STREAM`: For TCP (reliable, connection-oriented).
  - `SOCK_DGRAM`: For UDP (unreliable, connectionless).

- **Key Functions**:
  - `socket()`: Create a socket.
  - `bind()`: Assign an address and port to the socket.
  - `listen()`: Mark the socket as a passive socket (TCP only).
  - `accept()`: Accept incoming connections (TCP only).
  - `connect()`: Connect to a server.
  - `send()`/`recv()`: Send and receive data.
  - `sendto()`/`recvfrom()`: For connectionless communication.
  - `close()`: Close the socket.

- **Example: TCP Server**:
```c
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;
    char buffer[1024] = {0};
    const char *response = "Hello, Client!";

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(8080);

    bind(server_fd, (struct sockaddr *)&address, sizeof(address));
    listen(server_fd, 3);

    client_fd = accept(server_fd, NULL, NULL);
    read(client_fd, buffer, sizeof(buffer));
    printf("Received: %s\n", buffer);
    send(client_fd, response, strlen(response), 0);
    close(client_fd);
    close(server_fd);
    return 0;
}
```

- **Example: TCP Client**:
```c
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>

int main() {
    int sock;
    struct sockaddr_in server_address;
    const char *message = "Hello, Server!";
    char buffer[1024] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(8080);
    inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr);

    connect(sock, (struct sockaddr *)&server_address, sizeof(server_address));
    send(sock, message, strlen(message), 0);
    read(sock, buffer, sizeof(buffer));
    printf("Received: %s\n", buffer);
    close(sock);
    return 0;
}

```

---
### Signals in C/C++

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
---
### Threads in C/C++ (POSIX Threads - `pthread`)

POSIX Threads (`pthread`) provide a standard way to create and manage threads in Unix-like operating systems. Threads allow concurrent execution within a single process, sharing the same memory space.

#### Key Functions
- **Thread Creation**:
```c
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine(void *), void *arg);

// thread: Pointer to the thread identifier.
// attr: Attributes for the thread (NULL for default).
// start_routine: Function to execute in the thread.
// arg: Argument passed to the thread function.
```

- **Thread Joining**:
```c
// Waits for a thread to finish.
int pthread_join(pthread_t thread, void **retval);
```

- **Thread Termination**:
```c
// Terminates the calling thread.
void pthread_exit(void *retval);
```

- **Mutexes**:
```c
// For synchronizing access to shared resources.
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_lock(&mutex);
pthread_mutex_unlock(&mutex);
```

- **Key Points**:
  - Threads share global and heap memory but have separate stacks.
  - Mutexes prevent race conditions by ensuring exclusive access to critical sections.
  - pthread_join is used to wait for a thread to finish and retrieve its result.

- **Example**:
```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

void* thread_function(void* arg) {
    printf("Thread: Hello from thread %d!\n", *(int*)arg);
    pthread_exit(NULL);
}

int main() {
    pthread_t thread;
    int thread_arg = 1;

    if (pthread_create(&thread, NULL, thread_function, &thread_arg) != 0) {
        perror("Failed to create thread");
        return 1;
    }

    pthread_join(thread, NULL);
    printf("Main: Thread has finished.\n");
    return 0;
}
```

- **Example: Using Mutexes**:
```c
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int counter = 0;

void* increment_counter(void* arg) {
    for (int i = 0; i < 1000; i++) {
        pthread_mutex_lock(&mutex);
        counter++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main() {
    pthread_t threads[2];

    for (int i = 0; i < 2; i++) {
        if (pthread_create(&threads[i], NULL, increment_counter, NULL) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    for (int i = 0; i < 2; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("Final Counter Value: %d\n", counter);
    pthread_mutex_destroy(&mutex);
    return 0;
}

```


---
### Semaphores in C/C++

A **semaphore** is a synchronization primitive used to control access to a shared resource in concurrent programming. It is often used to avoid race conditions or ensure mutual exclusion.

#### Types of Semaphores
1. **Counting Semaphores**: Can have a value greater than one, useful for managing a pool of resources.
2. **Binary Semaphores**: Similar to a mutex, but with different ownership rules.

#### POSIX Semaphores

POSIX provides two types of semaphores:
1. **Named Semaphores**:
   - Persistent across processes.
   - Identified by a name.
2. **Unnamed Semaphores**:
   - Only valid within the process or among threads sharing memory.

#### Key Functions for Semaphores

- **Initialization**:
```c
#include <semaphore.h>
int sem_init(sem_t *sem, int pshared, unsigned int value);
```
  - `sem`: Pointer to the semaphore.
  - `pshared`: Non-zero for process-shared, zero for thread-shared.
  - `value`: Initial value of the semaphore.

- **Destroy**:
```c
int sem_destroy(sem_t *sem);
```

- **Wait (Decrement)**:
```c
int sem_wait(sem_t *sem);
```
- **Signal (Increment)**:
```c
int sem_post(sem_t *sem);
```
- **Try Wait**:
```c
int sem_trywait(sem_t *sem);
```

- **Example: Thread Synchronization Using Semaphores**:
```c
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>

sem_t semaphore;

void* thread_function(void* arg) {
    sem_wait(&semaphore); // Wait (decrement semaphore)
    printf("Thread %d is executing.\n", *(int*)arg);
    sleep(1); // Simulate work
    printf("Thread %d is done.\n", *(int*)arg);
    sem_post(&semaphore); // Signal (increment semaphore)
    return NULL;
}

int main() {
    pthread_t threads[3];
    int thread_ids[3] = {1, 2, 3};

    sem_init(&semaphore, 0, 1); // Initialize semaphore with value 1 (binary semaphore)

    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_function, &thread_ids[i]);
    }

    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }

    sem_destroy(&semaphore); // Destroy semaphore
    return 0;
}
```

- **Key Points**:
- Semaphores are suitable for controlling access to resources in multithreaded or multiprocess applications.
- Always destroy a semaphore after use to release system resources.
- Use semaphores judiciously to avoid deadlocks or resource contention.

---
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
---


### todo: 
```
string operation, makefile, shared memory, named semaphores, http, stat, grep
```
