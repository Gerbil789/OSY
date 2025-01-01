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
#include <pthread.h>
#include <sys/wait.h>
#include <semaphore.h>

#define STR_CLOSE "close"
#define STR_QUIT "quit"

//***************************************************************************
// log messages

#define LOG_ERROR 0 // errors
#define LOG_INFO 1  // information and notifications
#define LOG_DEBUG 2 // debug messages

// debug flag
int g_debug = LOG_INFO;

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

//***************************************************************************
// help

void help(int t_narg, char **t_args)
{
    if (t_narg <= 1 || !strcmp(t_args[1], "-h"))
    {
        printf(
            "\n"
            "  Socket server example.\n"
            "\n"
            "  Use: %s [-h -d] port_number\n"
            "\n"
            "    -d  debug mode \n"
            "    -h  this help\n"
            "\n",
            t_args[0]);

        exit(0);
    }

    if (!strcmp(t_args[1], "-d"))
        g_debug = LOG_DEBUG;
}

struct ThreadsPar
{
    int sock;
};

sem_t *g_sem_mutex = nullptr;

void *handleClient(void *t_par)
{

    ThreadsPar *l_ppar = (ThreadsPar *)t_par;
    int sock_client = l_ppar->sock;

    char message[500];
    char command[256];
    int readed = read(sock_client, command, sizeof(command));
    if (readed <= 0)
    {
        log_msg(LOG_ERROR, "chyba");
        exit(1);
    }

    char newCommand[150];
    strcpy(newCommand, command);

    char *execute = strtok(newCommand, " ");
    char *parOne = strtok(NULL, " ");
    char *parTwo = strtok(NULL, " ");

    if (strncasecmp("ls", command, 2) == 0)
    {
        if (fork() == 0)
        {
            dup2(sock_client, STDOUT_FILENO);
            execlp("ls", "ls", "/home/uherda/OSY/uherda_zadani/galerie", nullptr);
            // sleep(3);
            exit(1);
        }
        else
        {
            wait(nullptr);
        }
    }
    else if (strncasecmp("show", command, 4) == 0)
    {

        int child = fork();
        if (child == 0)
        {
            if (sem_wait(g_sem_mutex) < 0)
            {
                log_msg(LOG_ERROR, "Unable to enter into critical section!");
                close(sock_client);
                exit(1);
            }

            // char filename[50];

            // strcpy(filename, (&command[5]));

            // int len = strlen(filename);

            // filename[len - 2] = '\0';

            char path[150] = "/home/uherda/OSY/uherda_zadani/galerie/";

            strcat(path, parOne);

            dup2(sock_client, STDOUT_FILENO);
            execlp("display", "display", path, nullptr);

            exit(1);
        }
        else
        {
            int sleepTime = 0;
            if (parTwo)
            {
                sleepTime = atoi(parTwo);
                sleep(sleepTime);
                kill(child, SIGTERM);
            }

            wait(nullptr);

            char mes[] = "OK";
            write(sock_client, mes, sizeof(mes));

            if (sem_post(g_sem_mutex) < 0)
            {
                log_msg(LOG_ERROR, "Unable to unlock critical section!");
            }
        }
    }
    else if (strncasecmp("get", command, 3) == 0)
    {
        char filename[50];

        strcpy(filename, (&command[4]));

        // int len = strlen(filename);

        char path[150] = "/home/uherda/OSY/uherda_zadani/galerie/";

        strcat(path, parOne);

        FILE *image = fopen(path, "r");

        char buffer[500];
        int readed = fread(buffer, sizeof(char), sizeof(buffer), image);

        while (readed > 0)
        {
            write(sock_client, buffer, sizeof(buffer));
            readed = fread(buffer, sizeof(char), sizeof(buffer), image);
        }

        fclose(image);

        char mes[] = "OK";
        write(sock_client, mes, sizeof(mes));
    }
    else if (strncasecmp("put", command, 3) == 0)
    {

        char mes[] = "OK";
        write(sock_client, mes, sizeof(mes));

        char buffer[500];

        FILE *image = fopen("/home/uherda/OSY/uherda_zadani/galerie/outputPUT.JPG", "w");
        int readed = read(sock_client, buffer, sizeof(buffer));

        while (readed > 0)
        {

            fwrite(buffer, sizeof(char), readed, image);

            readed = read(sock_client, buffer, sizeof(buffer));
        }

        // fwrite(buffer, sizeof(char), sizeof(buffer), image);

        fclose(image);
    }
    else
    {
        sprintf(message, "Invalid command, closing...");
    }

    close(sock_client);
    pthread_exit((void *)((intptr_t)-1));
}

//***************************************************************************

int main(int t_narg, char **t_args)
{
    sem_unlink("/sem");
    // return 0;

    if (t_narg <= 1)
        help(t_narg, t_args);

     int l_port = 0;

    // parsing arguments
    for (int i = 1; i < t_narg; i++)
    {
        if (!strcmp(t_args[i], "-d"))
            g_debug = LOG_DEBUG;

        if (!strcmp(t_args[i], "-h"))
            help(t_narg, t_args);

        if (*t_args[i] != '-' && !l_port)
        {
            l_port = atoi(t_args[i]);
            break;
        }
    }

    if (l_port <= 0)
    {
        log_msg(LOG_INFO, "Bad or missing port number %d!", l_port);
        help(t_narg, t_args);
    }

    log_msg(LOG_INFO, "Server will listen on port: %d.", l_port);

    // socket creation
    int l_sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (l_sock_listen == -1)
    {
        log_msg(LOG_ERROR, "Unable to create socket.");
        exit(1);
    }

    g_sem_mutex = sem_open("/sem", O_RDWR | O_CREAT, 0660, 1);

    in_addr l_addr_any = {INADDR_ANY};
    sockaddr_in l_srv_addr;
    l_srv_addr.sin_family = AF_INET;
    l_srv_addr.sin_port = htons(l_port);
    l_srv_addr.sin_addr = l_addr_any;

    // Enable the port number reusing
    int l_opt = 1;
    if (setsockopt(l_sock_listen, SOL_SOCKET, SO_REUSEADDR, &l_opt, sizeof(l_opt)) < 0)
        log_msg(LOG_ERROR, "Unable to set socket option!");

    // assign port number to socket
    if (bind(l_sock_listen, (const sockaddr *)&l_srv_addr, sizeof(l_srv_addr)) < 0)
    {
        log_msg(LOG_ERROR, "Bind failed!");
        close(l_sock_listen);
        exit(1);
    }

    // listenig on set port
    if (listen(l_sock_listen, 1) < 0)
    {
        log_msg(LOG_ERROR, "Unable to listen on given port!");
        close(l_sock_listen);
        exit(1);
    }

    log_msg(LOG_INFO, "Enter 'quit' to quit server.");

    // go!
    while (1)
    {
        int l_sock_client = -1;

        // new client?
        sockaddr_in l_rsa;
        int l_rsa_size = sizeof(l_rsa);
        // new connection
        l_sock_client = accept(l_sock_listen, (sockaddr *)&l_rsa, (socklen_t *)&l_rsa_size);
        if (l_sock_client == -1)
        {
            log_msg(LOG_ERROR, "Unable to accept new client.");
            close(l_sock_listen);
            exit(1);
        }
        uint l_lsa = sizeof(l_srv_addr);
        // my IP
        getsockname(l_sock_client, (sockaddr *)&l_srv_addr, &l_lsa);
        log_msg(LOG_INFO, "My IP: '%s'  port: %d",
                inet_ntoa(l_srv_addr.sin_addr), ntohs(l_srv_addr.sin_port));
        // client IP
        getpeername(l_sock_client, (sockaddr *)&l_srv_addr, &l_lsa);
        log_msg(LOG_INFO, "Client IP: '%s'  port: %d",
                inet_ntoa(l_srv_addr.sin_addr), ntohs(l_srv_addr.sin_port));

        ThreadsPar par;
        par.sock = l_sock_client;

        pthread_t thread_id;
        int err = pthread_create(&thread_id, nullptr, handleClient, &par);
        if ( err )
            log_msg( LOG_INFO, "Unable to create thread.");
        else
            log_msg( LOG_DEBUG, "Thread created - system id 0x%X.", thread_id);

    } // while ( 1 )

    return 0;
}
