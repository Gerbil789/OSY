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
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>

#define MAX_CLIENTS 10

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

typedef struct
{
	int socket;
	int locked;
} Client;

// *** CRITICAL SECTION ***
sem_t client_list_sem;

int client_count = 0;
Client *clients[MAX_CLIENTS];
// ************************

void broadcast_message(const char *message, int sender_socket)
{
	// Add sender info to the message
	char formatted_message[1024];
	char sender_info[50];
	snprintf(sender_info, sizeof(sender_info), "[User %d]: ", sender_socket);
	snprintf(formatted_message, sizeof(formatted_message), "%s%s", sender_info, message);

	sem_wait(&client_list_sem); // Lock client list

	for (int i = 0; i < client_count; i++)
	{
		if (clients[i]->socket != sender_socket && !clients[i]->locked)
		{
			write(clients[i]->socket, formatted_message, strlen(formatted_message));
		}
	}

	sem_post(&client_list_sem); // Unlock client list
}

void *client_handler(void *arg)
{
	int client_socket = *(int *)arg;

	sem_wait(&client_list_sem);

	// Check if the server is full
	if (client_count >= MAX_CLIENTS)
	{
		// Send message to the clinet and close connection
		const char *msg = "Server full, try again later.\n";
		write(client_socket, msg, strlen(msg));
		close(client_socket);
		sem_post(&client_list_sem);
		return NULL;
	}

	// Add client to the list
	Client *new_client = (Client *)malloc(sizeof(Client));
	new_client->socket = client_socket;
	new_client->locked = 0;
	clients[client_count++] = new_client;
	sem_post(&client_list_sem);

	char buffer[256];
	int bytes_read;

	// Handle messages from the client
	while ((bytes_read = read(client_socket, buffer, sizeof(buffer))) > 0)
	{
		buffer[bytes_read] = '\0'; // Null terminate the message

		// If client sends LOCK or UNLOCK
		if (strncmp(buffer, "LOCK", 4) == 0)
		{
			sem_wait(&client_list_sem);
			new_client->locked = 5;
			sem_post(&client_list_sem);
		}
		else if (strncmp(buffer, "UNLOCK", 6) == 0)
		{
			sem_wait(&client_list_sem);
			new_client->locked = 0;
			sem_post(&client_list_sem);
		}
		else if (new_client->locked != 0)
		{
			sem_wait(&client_list_sem);
			new_client->locked--;
			sem_post(&client_list_sem);
		}
		else
		{
			broadcast_message(buffer, client_socket);
		}
	}

	// Client disconnected, remove from list
	sem_wait(&client_list_sem);
	for (int i = 0; i < client_count; i++)
	{
		if (clients[i]->socket == client_socket)
		{
			close(client_socket);
			free(clients[i]);
			for (int j = i; j < client_count - 1; j++)
			{
				clients[j] = clients[j + 1];
			}
			client_count--;
			break;
		}
	}
	sem_post(&client_list_sem);

	return NULL;
}

int main(int argc, char **argv)
{
	int port = 0;

	// Parse arguments
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-d"))
		{
			g_debug = LOG_DEBUG;
		}
		if (*argv[i] != '-' && !port)
		{
			port = atoi(argv[i]);
			break;
		}
	}

	if (port <= 0)
	{
		log_msg(LOG_INFO, "Bad or missing port number %d!", port);
		return 1;
	}

	log_msg(LOG_INFO, "Server will listen on port: %d.", port);

	// Initialize semaphore
	sem_init(&client_list_sem, 0, 1);

	// Create listening socket
	int listen_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (listen_socket == -1)
	{
		log_msg(LOG_ERROR, "Unable to create socket.");
		return 1;
	}

	// Set up server address
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = INADDR_ANY;

	// Bind socket
	if (bind(listen_socket, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
	{
		log_msg(LOG_ERROR, "Bind failed!");
		return 1;
	}

	// Listen for incoming connections
	if (listen(listen_socket, 10) < 0)
	{
		log_msg(LOG_ERROR, "Listen failed!");
		return 1;
	}

	log_msg(LOG_INFO, "Server is running. Waiting for connections...");

	// Accept clients
	while (1)
	{
		struct sockaddr_in client_addr;
		socklen_t client_len = sizeof(client_addr);
		int client_socket = accept(listen_socket, (struct sockaddr *)&client_addr, &client_len);
		if (client_socket < 0)
		{
			log_msg(LOG_ERROR, "Accept failed!");
			continue;
		}

		pthread_t thread_id;
		pthread_create(&thread_id, NULL, client_handler, (void *)&client_socket);
	}

	// Cleanup
	sem_destroy(&client_list_sem);
	return 0;
}