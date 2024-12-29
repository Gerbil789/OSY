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

#define LOG_ERROR 0 // errors
#define LOG_INFO 1	// information and notifications
#define LOG_DEBUG 2 // debug messages

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

typedef struct{
	int socket;
	int index;
} ClientArgs;

#define MAX_CLIENTS 4
int client_count = 0;
char sem_names[MAX_CLIENTS][32];
sem_t *semaphores[MAX_CLIENTS];

void send_msg(int socket, const char* msg)
{
	size_t bytes_send = write(socket, msg, strlen(msg));
	if(bytes_send < 0)
	{
		log_msg(LOG_ERROR, "failed to send message: %s", msg);
	}
}

void *client_handler(void *args)
{
	ClientArgs* clientArgs = (ClientArgs *)args;
	int client_socket = clientArgs->socket;
	int index = clientArgs->index;

	log_msg(LOG_INFO, "client socket: %d, index: %d", client_socket, index);

	size_t bytes_read;
	char buffer[64];

	int client_disconected = 0;

	while(1)
	{
		if(client_disconected == 1)
		{
			break;
		}
		sem_wait(semaphores[index]);

		for(int i = 0; i < 10; i++)
		{
			bytes_read = read(client_socket, buffer, sizeof(buffer));
			if(bytes_read < 0)
			{
				log_msg(LOG_ERROR, "failed to read data from client %d", client_socket);
				exit(1);
			}

			if(bytes_read == 0)
			{
				log_msg(LOG_INFO, "client %d disconected", client_socket);
				client_disconected = 1;
				break;
			}

			buffer[bytes_read] = '\0';

			log_msg(LOG_INFO, "Client %d [%d]: %s", client_socket, i, buffer);
			int a, b;
			char op;

			if(sscanf(buffer, "%d%c%d", &a, &op, &b) != 3)
			{
				log_msg(LOG_ERROR, "failed to parse client message.");
				continue;
			}

			//log_msg(LOG_DEBUG, "Parsed: a = %d, op = '%c', b = %d\n", a, op, b);

			char error_msg[32];
			int error = 0;
			float result;
			switch(op)
			{
				case '+':
					result = a + b;
					break;
				case '-':
					result = a - b;
					break;
				case '*':
					result = a * b;
					break;
				case '/':
					if(b == 0)
					{
						snprintf(error_msg, sizeof(error_msg), "cant divide by zero.");
						error = 1;
						break;
					}
					result = a / b;
					break;
				case '%':
					if(b == 0)
					{
						snprintf(error_msg, sizeof(error_msg), "cant mod by zero.");
						error = 1;
						break;
					}
					result = a % b;
					break;
				default:
					snprintf(error_msg, sizeof(error_msg), "invalid operator");
					error = 1;
					error = 1;
			}

			if(error)
			{
				send_msg(client_socket, error_msg);
				log_msg(LOG_INFO, error_msg);
			}
			else
			{
				char msg[32];
				snprintf(msg, sizeof(msg), "Result: %f\n", result);
				send_msg(client_socket, msg);
				log_msg(LOG_INFO, msg);
			}

			usleep(500000); //artificial delay :d
		}

		sem_post(semaphores[(index + 1) % client_count]);
	}
	close(client_socket);
	client_count--;
	return NULL;
}

int main(int t_narg, char **t_args)
{
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

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		sprintf(sem_names[i], "/%d_%d", l_port, i);
		sem_unlink(sem_names[i]);
		semaphores[i] = sem_open(sem_names[i], O_CREAT | O_EXCL, 0664, i == 0 ? 1 : 0);
		if(semaphores[i]  == SEM_FAILED)
		{
			log_msg(LOG_ERROR, "failed to create named semaphore: %s", sem_names[i]);
		}
	}

	log_msg(LOG_INFO, "Enter 'quit' to quit server.");

	while (1)
	{
		int l_sock_client = -1;

		// list of fd sources
		pollfd l_read_poll = {l_sock_listen, POLLIN, 0};


		while (1) // wait for new client
		{
			// select from fds
			int l_poll = poll(&l_read_poll, 1, -1);

			if (l_poll < 0)
			{
				log_msg(LOG_ERROR, "Function poll failed!");
				exit(1);
			}

			if (l_read_poll.revents & POLLIN)
			{ // new client?
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
				log_msg(LOG_INFO, "My IP: '%s'  port: %d", inet_ntoa(l_srv_addr.sin_addr), ntohs(l_srv_addr.sin_port));
				
				// client IP
				getpeername(l_sock_client, (sockaddr *)&l_srv_addr, &l_lsa);
				log_msg(LOG_INFO, "Client IP: '%s'  port: %d", inet_ntoa(l_srv_addr.sin_addr), ntohs(l_srv_addr.sin_port));

				if(client_count >= MAX_CLIENTS)
				{
					const char *msg = "Server is full.";
					write(l_sock_client, msg, strlen(msg));
					break;
				}



				pthread_t thread;
				ClientArgs args = {l_sock_client, client_count};
				if (pthread_create(&thread, NULL, client_handler, &args) != 0)
				{
					log_msg(LOG_ERROR, "Failed to create thread.");
					exit(1);
				}

				client_count++;
			}

		} // while wait for client
	} 

	for(int i = 0; i < MAX_CLIENTS; i++)
	{
		sem_destroy(semaphores[i]);
		sem_unlink(sem_names[i]);
	}
	return 0;
}