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
#include <sys/mman.h>
#include <semaphore.h>
#include <sys/wait.h>

#define MAX_CLIENTS 2 // Maximum number of clients to handle

#define SEM_MUTEX_NAME "/sem_mutex3"
#define SEM_COUNTER_NAME "/sem_counter3"

sem_t *g_sem_mutex = nullptr;
sem_t *g_sem_counter = nullptr;

typedef struct
{
    bool playerOneTurn = true;
    char board[5][5]; // Hrací pole
} GameBoard;

bool playerOneTurn = true;

#define LOG_ERROR 0 // errors
#define LOG_INFO 1  // information and notifications
#define LOG_DEBUG 2 // debug messages

// debug flag
int g_debug = LOG_INFO;
int childProcesses[2];
int l_sock_number = 0; // To keep track of the number of child processes
int l_sock_clients[MAX_CLIENTS];
pollfd l_read_poll[MAX_CLIENTS + 1]; // We will monitor two pipes and two client sockets
sem_t *sem_player1, *sem_player2;
GameBoard *gameBoard;
int l_sock_client = -1;
bool firtMove = true;

void clientCode();

void log_msg(int t_log_level, const char *t_form, ...)
{
    const char *out_fmt[] = {
        "ERR: (%d-%s) %s\n",
        "INF: %s\n",
        "DEB: %s\n"};

    if (t_log_level && t_log_level > g_debug)
        return;

    char l_buf[1024];
    va_list l_arg;
    va_start(l_arg, t_form);
    vsprintf(l_buf, t_form, l_arg);
    va_end(l_arg);

    switch (t_log_level)
    {
    case LOG_INFO:
    case LOG_DEBUG:
        fprintf(stdout, out_fmt[t_log_level], l_buf);
        break;

    case LOG_ERROR:
        fprintf(stderr, out_fmt[t_log_level], errno, strerror(errno), l_buf);
        break;
    }
}

int main(int t_narg, char **t_args)
{

    sem_player1 = sem_open("/sem_player1", O_CREAT, 0644, 1); // Player 1 starts unlocked
    sem_player2 = sem_open("/sem_player2", O_CREAT, 0644, 0); // Player 2 starts locked

    if (sem_player1 == SEM_FAILED || sem_player2 == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }
    int l_port = 0;

    for (int i = 1; i < t_narg; i++)
    {
        if (*t_args[i] != '-' && !l_port)
        {
            l_port = atoi(t_args[i]);
            break;
        }
    }

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
    if (listen(l_sock_listen, 5) < 0) // Listen for up to MAX_CLIENTS
    {
        perror("listen");
        close(l_sock_listen);
        exit(1);
    }

    fcntl(l_sock_listen, F_SETFL, O_NONBLOCK);

    int l_fd = shm_open("/shm_example", O_RDWR, 0660);
    if (l_fd < 0)
    {
        // log_msg( LOG_ERROR, "Unable to open file for shared memory." );
        l_fd = shm_open("/shm_example", O_RDWR | O_CREAT, 0660);
        if (l_fd < 0)
        {
            // log_msg( LOG_ERROR, "Unable to create file for shared memory." );
            exit(1);
        }
        ftruncate(l_fd, sizeof(GameBoard));
        // log_msg( LOG_INFO, "File created, this process is first" );
        // l_first = 1;
    }

    gameBoard = (GameBoard *)mmap(nullptr, sizeof(GameBoard), PROT_READ | PROT_WRITE,
                                  MAP_SHARED, l_fd, 0); //(GameBoard *)malloc(sizeof(GameBoard));
    for (int i = 0; i < 5; i++)
    {
        for (int j = 0; j < 5; j++)
        {
            gameBoard->board[i][j] = '.';
        }
    }

    gameBoard->playerOneTurn = true;

    l_read_poll[0].fd = STDIN_FILENO; // Pipe read end
    l_read_poll[0].events = POLLIN;

    while (1)
    {

        sockaddr_in l_rsa;
        int l_rsa_size = sizeof(l_rsa);
        l_sock_client = accept(l_sock_listen, (sockaddr *)&l_rsa, (socklen_t *)&l_rsa_size);
        if (l_sock_client == -1)
        {

            int l_poll = poll(l_read_poll, 3, 100);

            if (l_poll < 0)
            {
                usleep(100000);
                continue;
            }

            if (l_read_poll[0].revents & POLLIN)
            {
                char l_buf[256];
                int l_len = read(STDIN_FILENO, l_buf, sizeof(l_buf));

                if (strncmp(l_buf, "quit", 4))
                {
                    kill(childProcesses[0], SIGKILL); // Kill the child process
                    kill(childProcesses[1], SIGKILL); // Kill the child process
                    wait(NULL);
                    exit(0);
                }
            }
            usleep(100000); // Sleep for 100ms and retry
            continue;
        }

        char message[500];
        if (l_sock_number == 0)
        {
            sprintf(message, "Player 1 connected");
            write(STDOUT_FILENO, message, strlen(message));
        }
        else if (l_sock_number == 1)
        {
            sprintf(message, "Player 2 connected");
            write(STDOUT_FILENO, message, strlen(message));

            sprintf(message, "Vaše řada!");
            write(l_sock_clients[0], message, strlen(message));

            sprintf(message, "Počkejte!");
            write(l_sock_clients[1], message, strlen(message));
        }
        else
        {
            sprintf(message, "NO another  players");

            write(STDOUT_FILENO, message, strlen(message));
            write(l_sock_client, message, strlen(message));
            close(l_sock_client);
            continue;
        }

        // setting variable for client
        l_read_poll[l_sock_number + 1].fd = l_sock_client;
        l_read_poll[l_sock_number + 1].events = POLLIN;

        playerOneTurn = (l_sock_number == 0 ? true : false);

        l_sock_clients[l_sock_number] = l_sock_client;
        int l_child = fork();
        childProcesses[l_sock_number] = l_child;

        if (l_child < 0)
        {
            perror("fork");
            exit(1);
        }
        if (l_child != 0) // Parent process
        {
            l_sock_number = l_sock_number + 1;
        }
        else // Child process
        {
            close(l_sock_listen); // Child doesn't need the listening socket
            clientCode();
        }
    }

    close(l_sock_listen);
    return 0;
}

void clientCode()
{
    while (1)
    {
        if (!firtMove || !playerOneTurn)
        {
            if (firtMove)
            {
                char messag[] = "Počkejte na řadu!\n";
                write(l_sock_client, messag, sizeof(messag));
            }
            if (playerOneTurn)
            {
                sem_wait(sem_player1); // Player 1 waits for their turn
            }
            else
            {
                sem_wait(sem_player2); // Player 2 waits for their turn
            }
        }

        firtMove = false;

        char messag[] = "Vaše řada!\n";
        write(l_sock_client, messag, sizeof(messag));

        int l_poll = poll(l_read_poll, 3, -1);

        if (l_poll < 0)
        {
            usleep(100);
            continue;
        }

        char l_buf[256];

        if (l_read_poll[l_sock_number + 1].revents & POLLIN)
        {
            

            

            
            int l_len = read(l_sock_client, l_buf, sizeof(l_buf));

            if (gameBoard->playerOneTurn == playerOneTurn) // in shared memory is boolen if this process is on turn
            {

                char row;
                int rowNumber = -1;
                int column = -1;
                sscanf(l_buf, "%c-%d", &row, &column);

                switch (row)
                {
                case 'A':
                    rowNumber = 0;
                    break;
                case 'B':
                    rowNumber = 1;
                    break;
                case 'C':
                    rowNumber = 2;
                    break;
                case 'D':
                    rowNumber = 3;
                    break;
                case 'E':
                    rowNumber = 4;
                    break;

                default:
                    break;
                }

                if (column == -1 || rowNumber == -1 || column < 0 || column > 4 || rowNumber < 0 || rowNumber > 4)
                {

                    char errorInput[1000];
                    sprintf(errorInput, "Neplatný tah: '%s', prosím zadejte ve formátu [A-E]-[0-4], eg 'C-3'\n", l_buf);
                    write(l_sock_client, errorInput, strlen(errorInput));
                }
                else
                {
                    gameBoard->board[rowNumber][column] = (playerOneTurn ? 'X' : 'O');
                    gameBoard->playerOneTurn = !gameBoard->playerOneTurn;

                    char message[500];

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

                    write(l_sock_client, board_str, strlen(board_str));

                    if (playerOneTurn)
                    {
                        sem_post(sem_player2); // Unlock Player 2
                    }
                    else
                    {
                        sem_post(sem_player1); // Unlock Player 1
                    }

                    char messag2[] = "Počkejte na řadu!";
                    write(l_sock_client, messag2, sizeof(messag2));
                }
            }
            else
            {
                char messag[] = "not your turn!";
                write(l_sock_client, messag, sizeof(messag));
            }

            
           

            

            if (strncmp(l_buf, "quit", 4) == 0)
            {
                char messag[] = "client disconnect";
                write(l_sock_client, messag, sizeof(messag));
                close(l_sock_client);
                exit(0);
            }
        }
    }
}