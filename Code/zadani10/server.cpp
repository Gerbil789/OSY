//***************************************************************************
//
// Program example for labs in subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2017
//
// Example of socket server.
//
// This program is example of socket server and it allows to connect and serve
// the only one client.
// The mandatory argument of program is port number for listening.
//
//***************************************************************************

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <sys/types.h>

// int main(int t_narg, char **t_args)
// {

//     int l_port = 0;

//     for (int i = 1; i < t_narg; i++)
//     {
//         if (*t_args[i] != '-' && !l_port)
//         {
//             l_port = atoi(t_args[i]);
//             break;
//         }
//     }

//     // socket creation
//     int l_sock_listen = socket(AF_INET, SOCK_STREAM, 0);
//     if (l_sock_listen == -1)
//     {
//         exit(1);
//     }

//     in_addr l_addr_any = {INADDR_ANY};
//     sockaddr_in l_srv_addr;
//     l_srv_addr.sin_family = AF_INET;
//     l_srv_addr.sin_port = htons(l_port);
//     l_srv_addr.sin_addr = l_addr_any;

//     // Enable the port number reusing
//     int l_opt = 1;
//     if (setsockopt(l_sock_listen, SOL_SOCKET, SO_REUSEADDR, &l_opt, sizeof(l_opt)) < 0)
//     {
//         exit(1);
//         // log_msg( LOG_ERROR, "Unable to set socket option!" );
//     }

//     // assign port number to socket
//     if (bind(l_sock_listen, (const sockaddr *)&l_srv_addr, sizeof(l_srv_addr)) < 0)
//     {
//         // log_msg(LOG_ERROR, "Bind failed!");
//         close(l_sock_listen);
//         exit(1);
//     }

//     // listenig on set port
//     if (listen(l_sock_listen, 1) < 0)
//     {
//         // log_msg(LOG_ERROR, "Unable to listen on given port!");
//         close(l_sock_listen);
//         exit(1);
//     }

//     int l_sock_number = 1;
//     while (1)
//     {
//         int l_sock_client = -1;
//         pollfd l_read_poll[2];

//         sockaddr_in l_rsa;
//         int l_rsa_size = sizeof(l_rsa);
//         l_sock_client = accept(l_sock_listen, (sockaddr *)&l_rsa, (socklen_t *)&l_rsa_size);
//         if (l_sock_client == -1)
//         {

//             // log_msg(LOG_ERROR, "Unable to accept new client.");
//             close(l_sock_listen);
//             exit(1);
//         }

//         int l_mypipe[2];

//         if (pipe(l_mypipe) < 0)
//         {
//             // log_msg( LOG_ERROR, "Unable to create pipe!" );
//             exit(1);
//         }

//         // int l_mypipeSecond[ 2 ];

//         // if ( pipe( l_mypipeSecond ) < 0 )
//         // {
//         //    // log_msg( LOG_ERROR, "Unable to create pipe!" );
//         //     exit( 1 );
//         // }

//         l_read_poll[0].fd = l_mypipe[0];
//         l_read_poll[0].events = POLLIN;
//         l_read_poll[1].fd = l_sock_client;
//         l_read_poll[1].events = POLLIN;

//         int l_child = fork();
//         if (l_child < 0)
//         {
//             //  log_msg( LOG_ERROR, "Unable to create new process!" );
//             exit(1);
//         }

//         if (l_child != 0)
//         {                         // Parent process
//             // Parent should close the client socket
//           //  close(l_mypipe[1]);

//             bool run = true;
//             while (l_sock_number == 2)
//             {
//                 int l_poll = poll(l_read_poll, 2, -1);

//                 char l_buf[256];
//                 if (l_read_poll[0].revents & POLLIN)
//                 {
//                     int l_len = read(l_mypipe[0], l_buf, sizeof(l_buf));

//                     if (strncmp(l_buf, "quit", 4) == 0)
//                     {
//                        run = false;
//                        close(l_sock_client);
//                        break;
//                     }

//                     if (l_len > 0)
//                     {
//                         write(STDOUT_FILENO, l_buf, l_len);
//                         char mes[] = "test test pokus";
//                        // write(l_sock_client, mes, sizeof(mes));
//                     }

//                 }

//             }

//             l_sock_number = l_sock_number + 1;
//         }
//         else
//         { // Child process

//             close(l_sock_listen);
//             close(l_mypipe[0]);
//             while (1)
//             {
//                 int l_poll = poll(l_read_poll, 2, -1);

//                 if (l_poll < 0)
//                 {
//                     exit(1);
//                 }

//                 char l_buf[256];

//                 if (l_read_poll[l_sock_number].revents & POLLIN)
//                 {
//                     int l_len = read(l_sock_client, l_buf, sizeof(l_buf));

//                      if (l_len > 0)
//                     {
//                         write(l_mypipe[1], l_buf, l_len);
//                     }

//                     // If client sends "quit", disconnect
//                     if (strncmp(l_buf, "quit", 4) == 0)
//                     {
//                         char messag[] = "client disconnect";
//                         write(STDOUT_FILENO, messag, sizeof(messag));

//                         // Close only the client socket in the child process
//                         close(l_sock_client);
//                         exit(0); // Exit the child process
//                     }

//                 }
//             }
//         }

//     }

//     return 0;
// }

#define MAX_CLIENTS 2 // Maximum number of clients to handle

typedef struct
{
    char board[5][5]; // Hrací pole
} GameBoard;

int main(int t_narg, char **t_args)
{
    int l_port = 0;

    for (int i = 1; i < t_narg; i++)
    {
        if (*t_args[i] != '-' && !l_port)
        {
            l_port = atoi(t_args[i]);
            break;
        }
    }

    // socket creation
    int l_sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (l_sock_listen == -1)
    {
        perror("socket");
        exit(1);
    }

    in_addr l_addr_any = {INADDR_ANY};
    sockaddr_in l_srv_addr;
    l_srv_addr.sin_family = AF_INET;
    l_srv_addr.sin_port = htons(l_port);
    l_srv_addr.sin_addr = l_addr_any;

    // Enable the port number reusing
    int l_opt = 1;
    if (setsockopt(l_sock_listen, SOL_SOCKET, SO_REUSEADDR, &l_opt, sizeof(l_opt)) < 0)
    {
        perror("setsockopt");
        exit(1);
    }

    // assign port number to socket
    if (bind(l_sock_listen, (const sockaddr *)&l_srv_addr, sizeof(l_srv_addr)) < 0)
    {
        perror("bind");
        close(l_sock_listen);
        exit(1);
    }

    // listening on set port
    if (listen(l_sock_listen, MAX_CLIENTS) < 0) // Listen for up to MAX_CLIENTS
    {
        perror("listen");
        close(l_sock_listen);
        exit(1);
    }

    int l_sock_number = 0;       // To keep track of the number of child processes
    int l_pipes[MAX_CLIENTS][2]; // Array of pipes for each client
    int l_sock_clients[MAX_CLIENTS];
    pollfd l_read_poll[MAX_CLIENTS * 2]; // We will monitor two pipes and two client sockets

    GameBoard *gameBoard = (GameBoard *)malloc(sizeof(GameBoard));
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            gameBoard->board[i][j] = '.';
        }
    }

    while (l_sock_number < MAX_CLIENTS)
    {
        int l_sock_client = -1;
        sockaddr_in l_rsa;
        int l_rsa_size = sizeof(l_rsa);
        l_sock_client = accept(l_sock_listen, (sockaddr *)&l_rsa, (socklen_t *)&l_rsa_size);
        if (l_sock_client == -1)
        {
            perror("accept");
            close(l_sock_listen);
            exit(1);
        }

        if (pipe(l_pipes[l_sock_number]) < 0)
        {
            perror("pipe");
            exit(1);
        }

        // Set up the pollfd array to monitor the pipe and the client socket
        l_read_poll[2 * l_sock_number].fd = l_pipes[l_sock_number][0]; // Pipe read end
        l_read_poll[2 * l_sock_number].events = POLLIN;
        l_read_poll[2 * l_sock_number + 1].fd = l_sock_client; // Client socket
        l_read_poll[2 * l_sock_number + 1].events = POLLIN;

        l_sock_clients[l_sock_number] = l_sock_client;
        int l_child = fork();
        if (l_child < 0)
        {
            perror("fork");
            exit(1);
        }

        if (l_child != 0) // Parent process
        {
            // close(l_sock_client); // Parent should close the client socket
            while (l_sock_number == 1)
            {
                int l_poll = poll(l_read_poll, 2 * (l_sock_number + 1), -1); // Poll both pipes and client sockets

                if (l_poll < 0)
                {
                    perror("poll");
                    break;
                }

                char l_buf[256];
                for (int i = 0; i <= l_sock_number; i++)
                {
                    if (l_read_poll[2 * i].revents & POLLIN)
                    {
                        int l_len = read(l_pipes[i][0], l_buf, sizeof(l_buf));
                        if (l_len > 0)
                        {
                            write(STDOUT_FILENO, l_buf, l_len); // Print to stdout

                            char board_str[256];
                            int offset = 0;
                            for (int i = 0; i < 5; i++)
                            {
                                for (int j = 0; j < 5; j++)
                                {
                                    offset += snprintf(board_str + offset, sizeof(board_str) - offset, "%c ", gameBoard->board[i][j]);
                                }
                                offset += snprintf(board_str + offset, sizeof(board_str) - offset, "\n");
                            }
                            write(l_sock_clients[i], board_str, strlen(board_str));
                        }
                    }

                    if (l_read_poll[2 * i + 1].revents & POLLIN)
                    {
                    }
                }
            }
            l_sock_number = l_sock_number + 1;
            
        }
        else // Child process
        {
            close(l_sock_listen);             // Child doesn't need the listening socket
            close(l_pipes[l_sock_number][0]); // Child doesn't need to read from the pipe

            while (1)
            {
                int l_poll = poll(l_read_poll, 2 * (l_sock_number + 1), -1);

                if (l_poll < 0)
                {
                    perror("poll");
                    exit(1);
                }

                char l_buf[256];

                if (l_read_poll[2 * l_sock_number + 1].revents & POLLIN)
                {
                    int l_len = read(l_sock_client, l_buf, sizeof(l_buf));
                    if (l_len > 0)
                    {
                        write(l_pipes[l_sock_number][1], l_buf, l_len); // Write to the pipe
                    }

                    // If client sends "quit", disconnect
                    if (strncmp(l_buf, "quit", 4) == 0)
                    {
                        char messag[] = "client disconnect";
                        write(STDOUT_FILENO, messag, sizeof(messag));
                        close(l_sock_client); // Close the client socket
                        exit(0);              // Exit the child process
                    }
                }
            }
        }

        // Move to the next client and pipe
    }

    // Close the listening socket after the server is done
    close(l_sock_listen);

    return 0;
}