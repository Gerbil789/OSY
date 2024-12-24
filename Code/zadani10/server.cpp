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
        exit(1);
        // log_msg( LOG_ERROR, "Unable to set socket option!" );
    }

    // assign port number to socket
    if (bind(l_sock_listen, (const sockaddr *)&l_srv_addr, sizeof(l_srv_addr)) < 0)
    {
        // log_msg(LOG_ERROR, "Bind failed!");
        close(l_sock_listen);
        exit(1);
    }

    // listenig on set port
    if (listen(l_sock_listen, 1) < 0)
    {
        // log_msg(LOG_ERROR, "Unable to listen on given port!");
        close(l_sock_listen);
        exit(1);
    }

    while (1)
    {
        int l_sock_client = -1;
        pollfd l_read_poll[2];

        sockaddr_in l_rsa;
        int l_rsa_size = sizeof(l_rsa);
        l_sock_client = accept(l_sock_listen, (sockaddr *)&l_rsa, (socklen_t *)&l_rsa_size);
        if (l_sock_client == -1)
        {

            // log_msg(LOG_ERROR, "Unable to accept new client.");
            close(l_sock_listen);
            exit(1);
        }

        l_read_poll[0].fd = STDIN_FILENO;
        l_read_poll[0].events = POLLIN;
        l_read_poll[1].fd = l_sock_client;
        l_read_poll[1].events = POLLIN;

        while (1)
        {
            //TODO: proc poll funkce musi byt ve while?
            int l_poll = poll(l_read_poll, 2, -1);

            if (l_poll < 0)
            {
                exit(1);
            }

            char l_buf[256];
            if (l_read_poll[0].revents & POLLIN)
            {

                int l_len = read(STDIN_FILENO, l_buf, sizeof(l_buf));

                if (l_len > 0)
                {
                    write(l_sock_client, l_buf, l_len);
                }
            }

            if (l_read_poll[1].revents & POLLIN)
            {

                int l_len = read(l_sock_client, l_buf, sizeof(l_buf));
                if (l_len > 0)
                {
                    write(STDOUT_FILENO, l_buf, l_len);
                }
            }

        }
    }

    return 0;
}
