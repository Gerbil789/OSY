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
#include <semaphore.h>

#define STR_CLOSE "close"
#define STR_QUIT "quit"

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

void handle_client(int client_socket, const char *sem_name)
{
	sem_t *child_sem = sem_open(sem_name, 0);
	if (child_sem == SEM_FAILED) 
	{
		log_msg(LOG_ERROR, "sem_open in child failed");
    return;
	}

	char buffer[32];
	while(1)
	{
		size_t bytes_read = read(client_socket, buffer, sizeof(buffer));
		if(bytes_read < 0)
		{
			log_msg(LOG_ERROR, "failed to read from clinet");
			return;
		}

		buffer[bytes_read] = '\0';
		int sem_value = sem_getvalue(child_sem, &sem_value);
		if (sem_getvalue(child_sem, &sem_value) == -1)
		{
			log_msg(LOG_ERROR, "sem_getvalue failed");
    }

		
		if(strstr(buffer, "UP\n") != NULL)
		{
			log_msg(LOG_INFO, "Client %d requested UP   [%d]", client_socket, sem_value);
			sem_post(child_sem);
			const char* msg = "UP-OK\n";
			write(client_socket, msg, strlen(msg));
		}
		else if(strstr(buffer, "DOWN\n") != NULL)
		{
			log_msg(LOG_INFO, "Client %d requested DOWN [%d]", client_socket, sem_value);
			sem_wait(child_sem);
			const char* msg = "DOWN-OK\n";
			write(client_socket, msg, strlen(msg));
		}
		else
		{
			log_msg(LOG_INFO, "Client %d requested %s [%d]", client_socket, buffer, sem_value);
			const char* msg = "ERR\n";
			write(client_socket, msg, strlen(msg));
		}
	}
	sem_close(child_sem);
	close(client_socket);
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

	// create named semaphore
	char sem_name[10];
	snprintf(sem_name, sizeof(sem_name), "sem_%d", l_port);
	sem_unlink(sem_name);
	sem_t *sem = sem_open(sem_name, O_CREAT | O_EXCL, S_IRUSR | S_IWUSR, 2);
	if (sem == SEM_FAILED)
	{
		log_msg(LOG_ERROR, "sem_open failed");
		return 1;
	}
	else
	{
		log_msg(LOG_INFO, "Created named semaphore: %s", sem_name);
	}

	log_msg(LOG_INFO, "Enter 'quit' to quit server.");

	while (1)
	{
		int l_sock_client = -1;
		pollfd l_read_poll = {l_sock_listen, POLLIN, 0};

		while (1) // wait for new client
		{
			int l_poll = poll(&l_read_poll, 1, -1);

			if (l_poll < 0)
			{
				log_msg(LOG_ERROR, "Function poll failed!");
				exit(1);
			}

			if (l_read_poll.revents & POLLIN)
			{ 
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

				log_msg(LOG_INFO, "   [ Client connected ]   ");
				int pid = fork();

				if(pid < 0)
				{
					log_msg(LOG_ERROR, "fork failed");
					exit(1);
				}

				if(pid == 0)
				{
					// child process
					close(l_sock_listen);
					handle_client(l_sock_client, sem_name);
				}
				else
				{
					// parent process
				}
			}

		}
	}

	if (sem_close(sem) == -1)
	{
		perror("sem_close failed");
		return 1;
	}

	if (sem_unlink(sem_name) == -1)
	{
		perror("sem_unlink failed");
		return 1;
	}

	return 0;
}