
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
sem_t *g_sem_mutex = nullptr;

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

//***************************************************************************

int main(int t_narg, char **t_args)
{

    char test[] = "sdf;alkfj;aslkfdj;aslkfjsa;lkjfaslkjfdslfdsj";
    int size = strlen(test);

    char *string = (char *)malloc(sizeof(char) * size);

    char *token = strtok(test, ";");
    int i = 0;
    while (token != NULL)
    {
        string[i] = *token;
        i = i + strlen(token);
        token = strtok(NULL, ";");
        
    }

    int l_sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (l_sock_listen == -1)
    {
        log_msg(LOG_ERROR, "Unable to create socket.");
        exit(1);    
    } 

    in_addr l_addr_any = {INADDR_ANY};
    sockaddr_in l_srv_addr;
    l_srv_addr.sin_family = AF_INET;
    l_srv_addr.sin_port = htons(5500);
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

    sockaddr_in l_rsa;
    int l_rsa_size = sizeof(l_rsa);
    // new connection
    int l_sock_client = accept(l_sock_listen, (sockaddr *)&l_rsa, (socklen_t *)&l_rsa_size);
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


    write(l_sock_client, string, size);

    return 0;
}