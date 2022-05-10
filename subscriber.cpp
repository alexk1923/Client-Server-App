#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "helpers.h"
#include "protocols.h"
#include <sys/time.h>
#include <netinet/tcp.h>

void usage(char *file)
{
	fprintf(stderr, "Usage: %s client_id server_address server_port\n", file);
	exit(0);
}


int main(int argc, char *argv[])
{
	setvbuf(stdout, NULL, _IONBF, BUFSIZ);
	int sockfd, n, ret;
	struct sockaddr_in serv_addr;
	char buffer[BUFLEN];

	if (argc < 4) {
		usage(argv[0]);
	}

	buffer[0] = *(char *) argv[1];

    // set socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");


    // set server info
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");


	// connect with server
	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");

	// deactivate Nagle
	int yes = 1;
	int result = setsockopt(sockfd,
				IPPROTO_TCP,
				TCP_NODELAY,
				(char *) &yes, 
				sizeof(int));    // 1 - on, 0 - off
	DIE(result < 0, "Nagle");
	// send client ID to the server
	strncpy(buffer, argv[1], strlen(argv[1]) + 1);
	n = send(sockfd, buffer, strlen(buffer), 0);
	DIE(n < 0, "send");


	int read_size = 10;

	while (1) {

		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(sockfd, &read_fds);
		FD_SET(STDIN_FILENO, &read_fds);
		ret = select(sockfd + 1, &read_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
    
  		// read from user input
		if(FD_ISSET(STDIN_FILENO, &read_fds)) {
			memset(buffer, 0, sizeof(buffer));
		
			n = read(0, buffer, sizeof(buffer) - 1);
			DIE(n < 0, "read");

			if (strncmp(buffer, "exit", 4) == 0) {
				break;
			}

			char *token;

				int no_arg = 0;
				token = strtok(buffer, " ");

				char subscription_info[52];
				memset(subscription_info, 0, sizeof(subscription_info));
				int sf;
				if(strcmp(token, "subscribe") == 0) {
					while(token) {
						no_arg++;
						token = strtok(NULL, " ");		
						if(no_arg == 1) {
							strncpy(subscription_info, token, strlen(token) + 1);
						} else if(no_arg == 2) {
							sf = atoi(token);
						}	
					}

					if(no_arg != 3) {
						printf("Invalid number of arguments\n");
						continue;																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																														
					}

					int info_len = strlen(subscription_info);
					if(sf == 0) {
						subscription_info[info_len] = '0';
					} else {
						subscription_info[info_len] = '1';
					}
					subscription_info[info_len + 1] = 'S';
					subscription_info[info_len + 2] = '\0';
					
					
					n = send(sockfd, subscription_info, strlen(subscription_info), 0);
					DIE(n < 0, "send");	

					printf("Subscribed to topic.\n");
				} else if(strcmp(token, "unsubscribe") == 0) {
					token = strtok(NULL, " ");
					strncpy(subscription_info, token, strlen(token) + 1);
					int info_len = strlen(subscription_info);
					subscription_info[info_len] = 'U';
					subscription_info[info_len + 1] = '\0';

					token = strtok(NULL, " ");
					if(token) {
						printf("Invalid number of arguments for unsubscribe\n");
						continue;
					}
					n = send(sockfd, subscription_info, strlen(subscription_info), 0);
					DIE(n < 0, "send");
				} else {
					printf("Invalid command. Commands helper:\nExit - disconnect from server\n Subscribe <topic> <sf>\nUnsubscribe <topic>\n");
				}


		} else if(FD_ISSET(sockfd, &read_fds)) {
			// message from server
			memset(buffer, 0, sizeof(buffer));
			int m = recv(sockfd, buffer, read_size, 0);
			DIE(m < 0, "recv");

			if(strcmp(buffer, "close") == 0) {
				break;
			}

			// modify incoming read size
			if(read_size == 10) {
				int next_dim = atoi(buffer);
				read_size = next_dim;
				continue;
			}

			if(m != 0) { 
				printf("%s\n", buffer);
				// reset read size to default
				read_size = 10;
			}
		}
	}

	close(sockfd);

	return 0;
}
