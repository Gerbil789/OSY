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
