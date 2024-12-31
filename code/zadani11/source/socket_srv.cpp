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
#include <sys/wait.h>
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

#define SHARED_SEM "sem_mutex"
sem_t *sem_mutex;

void client_handler(int client_socket)
{
	char buffer[256];
	while (1)
	{
		// read command from client
		size_t bytes_read = read(client_socket, buffer, sizeof(buffer));
		if (bytes_read < 0)
		{
			log_msg(LOG_ERROR, "failed to read data from client");
			continue;
		}

		if (bytes_read == 0)
		{
			log_msg(LOG_INFO, "Client disconected");
			break;

		}
		buffer[bytes_read] = '\0';

		if(strstr(buffer, "exit") != NULL)
		{
			close(client_socket);
			log_msg(LOG_INFO, "Client exited");
			break;
		}

		// parse command
		int arg_count = 0;
		char *args[10]; // assume max number of arguments

		char *token = strtok(buffer, " \t\n");
		while (token != NULL && arg_count < 10)
		{
			args[arg_count++] = token;
			token = strtok(NULL, " \t\n");
		}
		args[arg_count] = NULL; // null-terminate the array, last argument in exec must be NULL

		if (arg_count == 0)
		{
			log_msg(LOG_INFO, "No command received.");
			continue;
		}

		// create pipe for redirect exec output
		int pipefd[2];
		if (pipe(pipefd) == -1)
		{
			log_msg(LOG_ERROR, "pipe failed");
			continue;
		}

		sem_wait(sem_mutex);

		log_msg(LOG_DEBUG, "Executing command: %s", args[0]);
		for (int i = 1; i < arg_count; ++i)
		{
			printf("Arg[%d]: %s\n", i, args[i]);
		}

		// create process for exec
		pid_t pid = fork();

		if (pid < 0)
		{
			log_msg(LOG_ERROR, "fork failed");
			close(pipefd[0]);		
			close(pipefd[1]);		
			sem_post(sem_mutex); 
			continue;
		}

		if (pid == 0)
		{
			// child
			close(pipefd[0]);								// close read end of the pipe
			dup2(pipefd[1], STDOUT_FILENO); // redirect stdout to pipe
			dup2(pipefd[1], STDERR_FILENO); // redirect stderr to pipe
			close(pipefd[1]);								// close write end of the pipe
			execvp(args[0], args);
			perror("exec failed");
			exit(1);
		}
		else
		{
			// parent
			close(pipefd[1]); // close write end of the pipe

			char output_buffer[1024];
			ssize_t n;
			while ((n = read(pipefd[0], output_buffer, sizeof(output_buffer) - 1)) > 0)
			{
				output_buffer[n] = '\0';
				send(client_socket, output_buffer, n, 0);
			}
			close(pipefd[0]); // close read end of the pipe
			wait(NULL);				// wait for the child process to finish
			log_msg(LOG_DEBUG, "Command finished: %s", args[0]);
			sem_post(sem_mutex);
		}
	}
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

	sem_unlink(SHARED_SEM);
	sem_mutex = sem_open(SHARED_SEM, O_CREAT | O_EXCL, 0644, 1);
	if (sem_mutex == SEM_FAILED)
	{
		log_msg(LOG_ERROR, "Failed to create named semaphore");
		return 1;
	}

	log_msg(LOG_INFO, "Enter 'quit' to quit server.");

	// go!
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

				int pid = fork();

				if (pid < 0)
				{
					log_msg(LOG_ERROR, "fork failed");
					return 1;
				}

				if (pid == 0)
				{
					// child
					close(l_sock_listen);
					client_handler(l_sock_client);
				}
				else
				{
					// parent
					close(l_sock_client);
				}
			}
		}
	}
	sem_close(sem_mutex);
	sem_unlink(SHARED_SEM);
	return 0;
}