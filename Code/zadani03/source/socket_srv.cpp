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

#define LOG_ERROR 0 // errors
#define LOG_INFO 1	// information and notifications
#define LOG_DEBUG 2 // debug messages

sem_t client_sem;

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

void *client_handler(void *arg)
{
	int client_socket = *(int *)arg;
	char buffer[256];
	int bytes_read = read(client_socket, buffer, sizeof(buffer));

	if (bytes_read <= 0)
	{
		log_msg(LOG_ERROR, "Failed to read from client");
		pthread_exit(NULL);
	}

	// Get LENGTH
	long length;
	if (sscanf(buffer, "%ld\n", &length) < 0)
	{
		log_msg(LOG_ERROR, "Failed to extract length from message.");
		pthread_exit(NULL);
	}
	log_msg(LOG_DEBUG, "Received length: %ld", length);

	sem_wait(&client_sem); // lock semaphore

	// sleep(5); // test multiple clients and semaphore

	// Respond with OK
	char response[] = "OK\n";
	size_t bytes_send = write(client_socket, response, strlen(response));
	if (bytes_send < strlen(response))
	{
		log_msg(LOG_ERROR, "Failed to send \"OK\" response");
		sem_post(&client_sem);
		pthread_exit(NULL);
	}

	// CREATE BUFFER
	char *dynamic_buffer = (char *)malloc(length);
	if (dynamic_buffer == NULL)
	{
		log_msg(LOG_ERROR, "Failed to allocate memory for the buffer.");
		sem_post(&client_sem); // Unlock semaphore before exiting
		pthread_exit(NULL);
	}

	// Read input file
	size_t total_bytes_read = 0;
	while (total_bytes_read < (size_t)length)
	{
		bytes_read = read(client_socket, dynamic_buffer + total_bytes_read, length - total_bytes_read);
		if (bytes_read <= 0)
		{
			log_msg(LOG_ERROR, "Failed to read data from client.");
			free(dynamic_buffer);	 // Free the buffer before exiting
			sem_post(&client_sem); // Unlock semaphore before exiting
			pthread_exit(NULL);
		}
		total_bytes_read += bytes_read;
	}
	log_msg(LOG_DEBUG, "Received %zu bytes of data.", total_bytes_read);

	// PROCESS DATA
	char *line_start = dynamic_buffer;
	int line_number = 1;
	char processed_data[1024]; // Buffer to hold processed data
	size_t processed_data_length = 0;

	while (line_start < dynamic_buffer + total_bytes_read)
	{
		// Find the end of the current line
		char *line_end = strchr(line_start, '\n');
		if (line_end == NULL)
			line_end = dynamic_buffer + total_bytes_read; // No newline, end of buffer

		// Format the line with the line number
		int len = snprintf(processed_data + processed_data_length, sizeof(processed_data) - processed_data_length, "%d: ", line_number);
		processed_data_length += len;

		// Copy the line content
		len = snprintf(processed_data + processed_data_length, sizeof(processed_data) - processed_data_length, "%.*s\n", (int)(line_end - line_start), line_start);
		processed_data_length += len;

		// Move to the next line
		line_start = line_end + 1;
		line_number++;
	}

	// Send the processed data back to the client
	bytes_send = write(client_socket, processed_data, processed_data_length);
	if (bytes_send < processed_data_length)
	{
		log_msg(LOG_ERROR, "Failed to send processed data to client.");
	}


	free(dynamic_buffer);	 	// Free the dynamically allocated buffer
	sem_post(&client_sem); 	// unlock semaphore
	close(client_socket);		// close socket
	pthread_exit(NULL);
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

	sem_init(&client_sem, 0, 1);

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

		// list of fd sources
		pollfd l_read_poll[2];
		l_read_poll[1].fd = l_sock_listen;
		l_read_poll[1].events = POLLIN;

		while (1) // wait for new client
		{
			int l_poll = poll(l_read_poll, 2, -1);

			if (l_poll < 0)
			{
				log_msg(LOG_ERROR, "Function poll failed!");
				exit(1);
			}

			// new client?
			if (l_read_poll[1].revents & POLLIN)
			{
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

				pthread_t thread;

				if (pthread_create(&thread, NULL, client_handler, (void *)&l_sock_client) != 0)
				{
					log_msg(LOG_ERROR, "Failed to create thread");
					exit(1);
				}

				// Detach the thread so it cleans up automatically when done
				pthread_detach(thread);
			}
		}
		sem_destroy(&client_sem);
		return 0;
	}
}