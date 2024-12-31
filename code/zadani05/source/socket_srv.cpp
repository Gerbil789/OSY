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
#include <errno.h>
#include <pthread.h>
#include <iostream>
#include <semaphore.h>
#include <sys/mman.h>
#include <cstring>

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

#define MAX_SPECTATORS 10
#define MAX_PLAYERS 2

typedef struct
{
	int socket;
	int index;
} PlayerArgs;

// **** CRITICAL SECTION ****
sem_t client_list_sem;

int player_count = 0;
int players[MAX_PLAYERS];

int spectator_count = 0;
int spectators[MAX_SPECTATORS];
// **************************

// **** SYNCHRONIZATION ****
sem_t sem_players[MAX_PLAYERS];
// *************************

const int board_size = 6;
char board[board_size][board_size];

int l_sock_listen;

void initialize_board()
{
	for (int i = 0; i < board_size; i++)
	{
		for (int j = 0; j < board_size; j++)
		{
			board[i][j] = '.';
		}
	}
}

char *create_board_buffer()
{
	int buffer_size = board_size * board_size + board_size + 1; // board size + "\n" characters +1 for null terminator
	char *buffer = (char *)malloc(buffer_size);

	int pos = 0;

	for (int i = 0; i < board_size; i++)
	{
		for (int j = 0; j < board_size; j++)
		{
			buffer[pos++] = board[i][j];
		}
		buffer[pos++] = '\n';
	}
	buffer[pos] = '\0';

	return buffer;
}

void send_msg(int socket, const char *msg)
{
	size_t bytes_sent = write(socket, msg, strlen(msg));
	if (bytes_sent < 0)
	{
		log_msg(LOG_ERROR, "Failed to send initial message to the client %d", socket);
	}
}

void send_board_to_spectators()
{
	char *buffer = create_board_buffer();
	for (int i = 0; i < spectator_count; i++)
	{
		send_msg(spectators[i], buffer);
	}
	printf("%s\n", buffer);
}

int process_move(const char *move, char symbol)
{
	//assume the move is in '<row>:<col>' and its only one letter/number 
	if(strlen(move) < 3 || move[1] != ':')
	{
		return -1;
	}

	int row = move[0] - 'A';
	int col = move[2] - '1';

	if (row < 0 || row >= board_size || col < 0 || col >= board_size)
	{
		return -1;
	}

	if(board[row][col] != '.')
	{
		return -1;
	}

	board[row][col] = symbol;

	return 0;
}

void *player_handler(void *args)
{
	PlayerArgs *playerArgs = (PlayerArgs *)args;
	int client_socket = playerArgs->socket;
	int index = playerArgs->index;
	char symbol = index == 0 ? 'x' : 'o';

	while (1)
	{
		send_msg(client_socket, "prosím o strpení\n");

		sem_wait(&sem_players[index]);

		send_msg(client_socket, create_board_buffer());

		send_msg(client_socket, "zadej tah\n");


		char buffer[16];
		while (1)
		{
			//wait for the player move
			ssize_t bytes_received = read(client_socket, buffer, sizeof(buffer) - 1);
			if (bytes_received < 0)
			{
				log_msg(LOG_ERROR, "Failed to read data from the client %d", client_socket);
				pthread_exit(NULL);
			}
			buffer[bytes_received] = '\0'; // null-terminate

			if (process_move(buffer, symbol) == 0)
			{
				break;
			}
			send_msg(client_socket, "Neplatný tah, zadej znovu.\n");
		}

		send_board_to_spectators();

		sem_post(&sem_players[!index]);
	}

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
	l_sock_listen = socket(AF_INET, SOCK_STREAM, 0);
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

	sem_init(&client_list_sem, 0, 1);
	sem_init(&sem_players[0], 0, 0);
	sem_init(&sem_players[1], 0, 0);

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
				uint l_lsa = sizeof(l_srv_addr);

				// my IP
				getsockname(l_sock_client, (sockaddr *)&l_srv_addr, &l_lsa);
				log_msg(LOG_INFO, "My IP: '%s'  port: %d", inet_ntoa(l_srv_addr.sin_addr), ntohs(l_srv_addr.sin_port));

				// client IP
				getpeername(l_sock_client, (sockaddr *)&l_srv_addr, &l_lsa);
				log_msg(LOG_INFO, "Client IP: '%s'  port: %d", inet_ntoa(l_srv_addr.sin_addr), ntohs(l_srv_addr.sin_port));

				sem_wait(&client_list_sem);

				if (player_count < MAX_PLAYERS)
				{
					pthread_t thread;
					PlayerArgs args = {l_sock_client, player_count};
					if (pthread_create(&thread, NULL, player_handler, &args) != 0)
					{
						log_msg(LOG_ERROR, "Failed to create thread.");
						exit(1);
					}

					player_count++;
					if (player_count == MAX_PLAYERS)
					{
						// start game
						initialize_board();
						// first player connected starts
						sem_post(&sem_players[0]);
					}
				}
				else if (spectator_count < MAX_SPECTATORS)
				{
					spectators[spectator_count++] = l_sock_client;
				}
				else
				{
					log_msg(LOG_INFO, "max clients reached");
				}
				sem_post(&client_list_sem);
			}
		}
	}

	sem_destroy(&client_list_sem);
	sem_destroy(&sem_players[0]);
	sem_destroy(&sem_players[1]);
	return 0;
}
