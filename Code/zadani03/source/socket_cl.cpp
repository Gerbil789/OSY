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
#include <netdb.h>

#define STR_CLOSE "close"

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
				"  Socket client example.\n"
				"\n"
				"  Use: %s [-h -d] ip_or_name port_number InFile OutFile\n"
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


void send_filesize(int t_sock, long t_size)
{
	log_msg(LOG_DEBUG, "Sending file size to server.");

	char l_buf[128];
	snprintf(l_buf, sizeof(l_buf), "%ld\n", t_size);
	int l_len = write(t_sock, l_buf, strlen(l_buf));
	if (l_len < 0)
	{
		log_msg(LOG_ERROR, "Unable to send data to server.");
		exit(1);
	}
	else
	{
		log_msg(LOG_DEBUG, "Sent %d bytes to server.", l_len);
	}

}

void wait_for_OK(int t_sock)
{
	log_msg(LOG_DEBUG, "Waiting for OK from server.");

	char l_buf[32];
	int l_len = read(t_sock, l_buf, sizeof(l_buf));
	if (!l_len)
	{
		log_msg(LOG_DEBUG, "Server closed socket.");
		exit(1);
	}
	else if (l_len < 0)
	{
		log_msg(LOG_ERROR, "Unable to read data from server.");
		exit(1);
	}

	l_buf[l_len] = 0;
	if (strcmp(l_buf, "OK\n") != 0)
	{
		log_msg(LOG_ERROR, "Server responded with \"%s\".", l_buf);
		exit(1);
	}
	else
	{
		log_msg(LOG_DEBUG, "Server responded with \"OK\".");
	}
}


void send_file(int t_sock, FILE *t_file)
{
	log_msg(LOG_DEBUG, "Sending file to server.");

	char l_buf[1024];
	int l_len;
	fseek(t_file, 0, SEEK_SET);
	while ((l_len = fread(l_buf, 1, sizeof(l_buf), t_file)) > 0)
	{
		int l_sent = write(t_sock, l_buf, l_len);
		if (l_sent < 0)
		{
			log_msg(LOG_ERROR, "Unable to send data to server.");
			exit(1);
		}
		else if (l_sent < l_len)
		{
			log_msg(LOG_ERROR, "Sent only %d bytes from %d bytes.", l_sent, l_len);
			exit(1);
		}
	}

	log_msg(LOG_DEBUG, "File sent to server.");
}

void read_file(int t_sock, FILE *t_file)
{
	log_msg(LOG_DEBUG, "Reading file from server.");

	char l_buf[1024];
	int l_len;
	while ((l_len = read(t_sock, l_buf, sizeof(l_buf))) > 0)
	{
		int l_written = fwrite(l_buf, 1, l_len, t_file);
		if (l_written < 0)
		{
			log_msg(LOG_ERROR, "Unable to write to file.");
			exit(1);
		}
		else if (l_written < l_len)
		{
			log_msg(LOG_ERROR, "Written only %d bytes from %d bytes.", l_written, l_len);
			exit(1);
		}
	}
}


int main(int t_narg, char **t_args)
{

	if (t_narg <= 2)
		help(t_narg, t_args);

	int l_port = 0;
	char *l_host = nullptr;
	char *in_filename = nullptr;
	char *out_file_name = nullptr;

	for (int i = 1; i < t_narg; i++)
	{
		if (!strcmp(t_args[i], "-d"))
			g_debug = LOG_DEBUG;

		if (!strcmp(t_args[i], "-h"))
			help(t_narg, t_args);

		if (*t_args[i] != '-')
		{
			if (!l_host)
				l_host = t_args[i];
			else if (!l_port)
				l_port = atoi(t_args[i]);
			else if (!in_filename)
				in_filename = t_args[i];
			else if (!out_file_name)
				out_file_name = t_args[i];
		}
	}

	if (!l_host || !l_port || !in_filename || !out_file_name)
	{
		log_msg(LOG_INFO, "Host, port, input file, or output file is missing!");
		help(t_narg, t_args);
		exit(1);
	}

	// open input file
	FILE *in_file = fopen(in_filename, "rb");
	if (!in_file)
	{
		log_msg(LOG_ERROR, "Unable to open file: %s", in_filename);
		exit(1);
	}
	fseek(in_file, 0, SEEK_END);
	long in_file_size = ftell(in_file);

	log_msg(LOG_INFO, "Input file '%s' size: %ld bytes.", in_filename, in_file_size);

	// open output file
	FILE *out_file = fopen(out_file_name, "wb");
	if (!out_file)
	{
		log_msg(LOG_ERROR, "Unable to open file: %s", out_file_name);
		fclose(in_file);
		exit(1);
	}

	log_msg(LOG_INFO, "Connection to '%s':%d.", l_host, l_port);
	addrinfo l_ai_req, *l_ai_ans;
	bzero(&l_ai_req, sizeof(l_ai_req));
	l_ai_req.ai_family = AF_INET;
	l_ai_req.ai_socktype = SOCK_STREAM;

	int l_get_ai = getaddrinfo(l_host, nullptr, &l_ai_req, &l_ai_ans);
	if (l_get_ai)
	{
		log_msg(LOG_ERROR, "Unknown host name!");
		exit(1);
	}

	sockaddr_in l_cl_addr = *(sockaddr_in *)l_ai_ans->ai_addr;
	l_cl_addr.sin_port = htons(l_port);
	freeaddrinfo(l_ai_ans);

	// socket creation
	int l_sock_server = socket(AF_INET, SOCK_STREAM, 0);
	if (l_sock_server == -1)
	{
		log_msg(LOG_ERROR, "Unable to create socket.");
		exit(1);
	}

	// connect to server
	if (connect(l_sock_server, (sockaddr *)&l_cl_addr, sizeof(l_cl_addr)) < 0)
	{
		log_msg(LOG_ERROR, "Unable to connect server.");
		exit(1);
	}

	uint l_lsa = sizeof(l_cl_addr);

	// my IP
	getsockname(l_sock_server, (sockaddr *)&l_cl_addr, &l_lsa);
	log_msg(LOG_INFO, "My IP: '%s'  port: %d",inet_ntoa(l_cl_addr.sin_addr), ntohs(l_cl_addr.sin_port));
	
	// server IP
	getpeername(l_sock_server, (sockaddr *)&l_cl_addr, &l_lsa);
	log_msg(LOG_INFO, "Server IP: '%s'  port: %d", inet_ntoa(l_cl_addr.sin_addr), ntohs(l_cl_addr.sin_port));

	// send file size to server
	send_filesize(l_sock_server, in_file_size);

	// wait for "OK\n" from server
	wait_for_OK(l_sock_server);

	// send input file to server
	send_file(l_sock_server, in_file);
	fclose(in_file);

	// read output file from server
	read_file(l_sock_server, out_file);
	fclose(out_file);

	log_msg(LOG_INFO, "File transfer completed.");

	// close socket
	close(l_sock_server);
	return 0;
}