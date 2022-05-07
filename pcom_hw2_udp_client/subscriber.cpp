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
#include <sys/time.h>

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

    // setare campuri server
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(atoi(argv[3]));
	ret = inet_aton(argv[2], &serv_addr.sin_addr);
	DIE(ret == 0, "inet_aton");

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



			// se trimite mesaj la server
			n = send(sockfd, buffer, strlen(buffer), 0);
			DIE(n < 0, "send");
		}


		// if(FD_ISSET(sockfd, &read_fds)) {
		// 	memset(buffer, 0, BUFLEN);
		// 	int m = recv(sockfd, buffer, sizeof(buffer) - 1, 0);
		// 	DIE(m < 0, "recv");

		// 	if(m != 0) {	
		// 		printf ("S-a primit mesajul: %s\n", buffer);
		// 	}
		// }

		if (strncmp(buffer, "exit", 4) == 0) {
			break;
		}

	}

	close(sockfd);

	return 0;
}
