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
sockaddr_in g_srv_addr;
sem_t sem;

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

void execute_convert(const char *image_path, uint16_t x, uint16_t y)
{
	if (x > 0 && y > 0)
	{
		char resolution[16];
		snprintf(resolution, sizeof(resolution), "%dx%d", x, y);
		execlp("convert", "convert", "-resample", resolution, image_path, "-", NULL);
	}
	else
	{
		execlp("convert", "convert", image_path, "-", NULL);
	}
}

void exit_with_error(int client_socket, const char *msg)
{
	log_msg(LOG_ERROR, msg);
	close(client_socket);
	exit(1);
}

void handle_client(int l_sock_client)
{
	uint l_lsa = sizeof(g_srv_addr);
	getsockname(l_sock_client, (sockaddr *)&g_srv_addr, &l_lsa);
	log_msg(LOG_INFO, "My IP: '%s'  port: %d", inet_ntoa(g_srv_addr.sin_addr), ntohs(g_srv_addr.sin_port));
	getpeername(l_sock_client, (sockaddr *)&g_srv_addr, &l_lsa);
	log_msg(LOG_INFO, "Client IP: '%s'  port: %d", inet_ntoa(g_srv_addr.sin_addr), ntohs(g_srv_addr.sin_port));

	char l_buf[1024];
	int l_len = read(l_sock_client, l_buf, sizeof(l_buf));
	if (l_len <= 0)
	{
		exit_with_error(l_sock_client, "Client disconnected.");
	}

	l_buf[l_len] = '\0'; // Null-terminate the received data
	log_msg(LOG_DEBUG, "Received %d bytes.", l_len);

	if (strstr(l_buf, "GET") == NULL)
	{
		exit_with_error(l_sock_client, "Buffer does not contain 'GET'.");
	}

	if (strstr(l_buf, "HTTP") == NULL)
	{
		exit_with_error(l_sock_client, "Buffer does not contain 'HTTP'.");
	}

	// Parse the buffer
	int level = -1, x = 0, y = 0;
	if (sscanf(l_buf, "GET /level-%d/%dx%d HTTP", &level, &x, &y) < 1 && level < 0)
	{
		exit_with_error(l_sock_client, "Parsing failed.");
	}

	log_msg(LOG_DEBUG, "[level: %d, x: %d, y: %d]", level, x, y);

	if (level > 100 || level < 0)
	{
		exit_with_error(l_sock_client, "level is outside the range.");
	}

	int battery_level = (level / 20) * 20;
	char image_path[256];
	snprintf(image_path, sizeof(image_path), "../images/battery-%03d.png", battery_level);
	log_msg(LOG_DEBUG, "image path: %s", image_path);

	// Check if image exists
	if (access(image_path, F_OK) != 0)
	{
		exit_with_error(l_sock_client, "Image file not found.");
	}

	const char *header =
			"HTTP/1.1 200 OK\n"
			"Server: OSY/1.1.1 (Ubuntu)\n"
			"Accept-Ranges: bytes\n"
			"Vary: Accept-Encoding\n"
			"Content-Type: image/png\n\n";

	if (write(l_sock_client, header, strlen(header)) < 0)
	{
		exit_with_error(l_sock_client, "Failed to send HTTP header.");
	}

	sem_wait(&sem);

	pid_t pid = fork();

	if (pid == 0)
	{
		// Child process
		dup2(l_sock_client, STDOUT_FILENO); // Redirect STDOUT to socket
		close(l_sock_client);
		execute_convert(image_path, x, y);

		// If execlp fails, handle the error
		if (errno != 0)
		{
			log_msg(LOG_ERROR, "Failed to execute convert: %s", strerror(errno));
			exit(1);
		}
	}
	else if (pid > 0)
	{
		// Parent process: Wait for child to finish
		waitpid(pid, NULL, 0);
		sem_post(&sem); // Release semaphore
	}
	else
	{
		log_msg(LOG_ERROR, "Fork failed.");
		sem_post(&sem);
	}
	close(l_sock_client);
	exit(0); // Exit the child process
}

int main(int t_narg, char **t_args)
{
	int l_port = 0;
	for (int i = 1; i < t_narg; i++)
	{
		if (!strcmp(t_args[i], "-d"))
			g_debug = LOG_DEBUG;

		if (*t_args[i] != '-' && !l_port)
		{
			l_port = atoi(t_args[i]);
			break;
		}
	}

	if (l_port <= 0)
	{
		log_msg(LOG_INFO, "Bad or missing port number %d!", l_port);
	}

	// Binary semaphore (0: process-shared, 1: initial value)
	if (sem_init(&sem, 0, 1) != 0)
	{
		log_msg(LOG_ERROR, "Failed to initialize semaphore.");
		exit(1);
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
	g_srv_addr.sin_family = AF_INET;
	g_srv_addr.sin_port = htons(l_port);
	g_srv_addr.sin_addr = l_addr_any;

	// Enable the port number reusing
	int l_opt = 1;
	if (setsockopt(l_sock_listen, SOL_SOCKET, SO_REUSEADDR, &l_opt, sizeof(l_opt)) < 0)
		log_msg(LOG_ERROR, "Unable to set socket option!");

	// assign port number to socket
	if (bind(l_sock_listen, (const sockaddr *)&g_srv_addr, sizeof(g_srv_addr)) < 0)
	{
		log_msg(LOG_ERROR, "Bind failed!");
		close(l_sock_listen);
		exit(1);
	}

	// listenig on set port
	if (listen(l_sock_listen, 10) < 0)
	{
		log_msg(LOG_ERROR, "Unable to listen on given port!");
		close(l_sock_listen);
		exit(1);
	}

	log_msg(LOG_INFO, "Enter 'quit' to quit server.");

	while (1)
	{
		sockaddr_in l_rsa;
		socklen_t l_rsa_size = sizeof(l_rsa);

		int l_sock_client = accept(l_sock_listen, (sockaddr *)&l_rsa, &l_rsa_size);
		if (l_sock_client == -1)
		{
			log_msg(LOG_ERROR, "Unable to accept new client.");
			continue; // Continue to handle the next client
		}

		pid_t pid = fork();

		if (pid < 0)
		{
			log_msg(LOG_ERROR, "Fork failed!");
			close(l_sock_client);
			continue; // Continue to handle the next client
		}

		if (pid == 0)
		{
			// Child process: handle the client
			close(l_sock_listen); // Close the listening socket in the child process
			handle_client(l_sock_client);
		}
		else
		{
			// Parent process: close the client socket and continue listening
			close(l_sock_client);

			// Reap zombie processes
			while (waitpid(-1, NULL, WNOHANG) > 0);
		}
	}

	close(l_sock_listen);
	sem_destroy(&sem);
	return 0;
}