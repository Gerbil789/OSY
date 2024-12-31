# Threads in C/C++ (POSIX Threads - `pthread`)

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