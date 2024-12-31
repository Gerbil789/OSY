# Sockets in C/C++

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