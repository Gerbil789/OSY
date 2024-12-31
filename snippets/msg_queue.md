# Message Queues

Message queues are a form of inter-process communication (IPC) that allow processes to send and receive messages. They are particularly useful for synchronizing processes or exchanging data between them.

## What Are Message Queues?
A message queue is a data structure maintained by the operating system that stores messages. Processes can write messages to the queue and read messages from it. The operating system ensures that the messages are delivered in a predictable manner, often FIFO (First In, First Out).

## Key Concepts
- **Queue Identifier**: Each message queue is identified by a unique key.
- **Message**: A unit of data sent or received by a process.
- **Permissions**: Access to the queue is controlled by permissions.

## Using Message Queues in C/C++
In Unix-like systems, the POSIX and System V APIs are commonly used for message queues. Below, we'll focus on the System V message queue API.

### Including Required Headers

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
```

### Key Functions
- `msgget`: Creates or accesses a message queue.
- `msgsnd`: Sends a message to the queue.
- `msgrcv`: Receives a message from the queue.
- `msgctl`: Controls message queue operations (e.g., deletion).

### Example: Sending and Receiving Messages

#### Message Structure

Define a message structure for communication:

```c
typedef struct {
    long mtype; // Message type
    char mtext[100]; // Message text
} message_t;
```

#### Sender Program

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    long mtype;
    char mtext[100];
} message_t;

int main() {
    key_t key = ftok("msgqueue", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    message_t msg;
    msg.mtype = 1; // Message type
    strcpy(msg.mtext, "Hello, World!");

    if (msgsnd(msgid, &msg, sizeof(msg.mtext), 0) == -1) {
        perror("msgsnd");
        return 1;
    }

    printf("Message sent: %s\n", msg.mtext);
    return 0;
}
```

#### Receiver Program

```c
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct {
    long mtype;
    char mtext[100];
} message_t;

int main() {
    key_t key = ftok("msgqueue", 65);
    int msgid = msgget(key, 0666 | IPC_CREAT);

    message_t msg;

    if (msgrcv(msgid, &msg, sizeof(msg.mtext), 1, 0) == -1) {
        perror("msgrcv");
        return 1;
    }

    printf("Message received: %s\n", msg.mtext);

    // Remove the message queue
    if (msgctl(msgid, IPC_RMID, NULL) == -1) {
        perror("msgctl");
        return 1;
    }

    return 0;
}
```

### Explanation
1. **Key Creation**: The `ftok` function generates a unique key based on a file path and an identifier.
2. **Queue Creation**: The `msgget` function creates a new message queue or accesses an existing one.
3. **Sending Messages**: The `msgsnd` function sends a message to the queue.
4. **Receiving Messages**: The `msgrcv` function retrieves a message from the queue.
5. **Cleanup**: The `msgctl` function removes the message queue when it is no longer needed.

## Notes
- Ensure proper synchronization when multiple processes access the same queue.
- Handle errors from IPC functions carefully to avoid undefined behavior.
- Use unique keys for different queues to avoid collisions.

## Advantages of Message Queues
- Decouples processes, allowing them to run independently.
- Provides reliable communication between processes.
- Supports message prioritization via `mtype`.

## Disadvantages
- Limited message size.
- Can become a bottleneck if overused.
- Requires careful handling of permissions and cleanup.
