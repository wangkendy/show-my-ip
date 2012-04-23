#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <time.h>

#define SIZE 8192
#define PORT_NO 12345

int main()
{
	int server_sockfd, client_sockfd;
	struct sockaddr_in server_address, client_address;
	int client_len;
	int fd_size, ret;
	int optval;
	char buf[SIZE];
	fd_set readfds, testfds;
	FILE *log_file;

	log_file = fopen("telnet_ip.log", "a");
	if (log_file == NULL) {
		perror("fopen()");
		exit(1);
	}

	server_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_sockfd < 0) {
		perror("socket()");
		exit(1);
	}
	optval = 1;
	if (setsockopt(server_sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
		perror("setsockopt()");
		exit(1);
	}

	server_address.sin_family = AF_INET;
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(PORT_NO);
	if (bind(server_sockfd, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) {
		perror("bind()");
		exit(1);
	}

	if (listen(server_sockfd, 5) < 0) {
		perror("listen()");
		exit(1);
	}

	fd_size = server_sockfd + 1;
	FD_ZERO(&readfds);
	FD_SET(server_sockfd, &readfds);

	while(1) {
		testfds = readfds;
		ret = select(fd_size, &testfds, 0, 0, 0);
		if (ret < 0) {
			perror("select()");
			exit(1);
		}
		if (ret == 0) continue;
		
		if (FD_ISSET(server_sockfd, &testfds)) {
			client_sockfd = accept(server_sockfd, (struct sockaddr *)&client_address, &client_len);
			if (client_sockfd < 0) {
				perror("accept()");
				exit(1);
			}
			time_t t = time(0);
			struct tm *tm = localtime(&t);
			char *ip_addr;
			ip_addr = (char *)inet_ntoa(client_address.sin_addr);
			fprintf(log_file, "%d-%02d-%02d %02d:%02d:%02d %s\n", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec, ip_addr);
			fflush(log_file);
			sprintf(buf, "Your IP address is %s\n", ip_addr);
			write(client_sockfd, buf, strlen(buf));
			close(client_sockfd);
		}
	}

	return 0;
}
