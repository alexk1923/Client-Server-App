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

    // setare socket
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	DIE(sockfd < 0, "socket");

      // handle the error

    // setare campuri server
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

	 int yes = 1;
	int result = setsockopt(sockfd,
			IPPROTO_TCP,
			TCP_NODELAY,
			(char *) &yes, 
			sizeof(int));    // 1 - on, 0 - off

	ret = connect(sockfd, (struct sockaddr*) &serv_addr, sizeof(serv_addr));
	DIE(ret < 0, "connect");


	strncpy(buffer, argv[1], strlen(argv[1]) + 1);
	n = send(sockfd, buffer, strlen(buffer), 0);
	DIE(n < 0, "send");


	

	while (1) {

		fd_set read_fds;
		FD_ZERO(&read_fds);
		FD_SET(sockfd, &read_fds);
		FD_SET(STDIN_FILENO, &read_fds);
		ret = select(sockfd + 1, &read_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
    
  		// se citeste de la stdin

		if(FD_ISSET(STDIN_FILENO, &read_fds)) {
			memset(buffer, 0, sizeof(buffer));
		
			n = read(0, buffer, sizeof(buffer) - 1);
			DIE(n < 0, "read");

			char *token;

				int no_arg = 0;
				token = strtok(buffer, " ");
				// printf("token =%s|\n", token);

				char subscription_info[52];
				memset(subscription_info, 0, sizeof(subscription_info));
				int sf;
				if(strcmp(token, "subscribe") == 0) {
					// printf("Subscribed to topic.\n");
					while(token) {
						no_arg++;
						token = strtok(NULL, " ");		
						if(no_arg == 1) {
							// printf("topic = %s\n", token);
							strncpy(subscription_info, token, strlen(token) + 1);
						} else if(no_arg == 2) {
							// printf("sf = %s", token);
							sf = atoi(token);   
						}	
					}

					if(no_arg != 3) {
						printf("Invalid number of arguments\n");
						continue;																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																														
					}

					// se trimite mesaj la server

					// char subscription_info[52];
					// strncpy(subscription_info, topic_subscribed, strlen(topic_subscribed) + 1);
					int info_len = strlen(subscription_info);
					if(sf == 0) {
						strncpy(&subscription_info[info_len], "0", 1);
						// subscription_info[strlen(subscription_info)] = '0';
					} else {
						strncpy(&subscription_info[info_len], "1", 1);
						// subscription_info[strlen(subscription_info)] = '1';
					}
					strncpy(&subscription_info[info_len + 1], "S", 1);
					subscription_info[info_len + 2] = '\0';

					
					int yes = 1;
					int result = setsockopt(sockfd,
								IPPROTO_TCP,
								TCP_NODELAY,
								(char *) &yes, 
								sizeof(int));    // 1 - on, 0 - off
					n = send(sockfd, subscription_info, strlen(subscription_info), 0);
					// DIE(n < 0, "send");	

					printf("Subscribed to topic.\n");
				}


		} else if(FD_ISSET(sockfd, &read_fds)) {
			memset(buffer, 0, sizeof(buffer));
			int m = recv(sockfd, buffer, 1600, 0);
			DIE(m < 0, "recv");
			
			if(strcmp(buffer, "close") == 0) {
				break;
			}
			// printf("m = %d\n", m);
			// printf("buff: %s\n", buffer);

			if(m != 0) {
				message_udp *new_msg = (message_udp *)buffer;
				// <IP_CLIENT_UDP>:<PORT_CLIENT_UDP> - <TOPIC> - <TIP_DATE> - <VALOARE_MESAJ>
				printf("%s:%s - %s - %s - %s\n", new_msg->ip_udp, new_msg->port_udp, new_msg->topic, new_msg->data_type, new_msg->payload);
				// print_udp_msg(*new_msg);
				// printf("%s\n", buffer);
			}
		}
		
		// printf("lala");
		if (strncmp(buffer, "exit", 4) == 0) {
			break;
		}
	}

	close(sockfd);

	return 0;
}
