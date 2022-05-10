#include <iostream>
#include<vector>
using namespace std;
#include <bits/stdc++.h>
#include <unordered_map>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "helpers.h"
#include "protocols.h"
#include <sys/time.h>
#include <netinet/tcp.h>


void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

int countDigits(int x) {
    int c = 0;
    int xx = x;
    while(xx) {
        xx /= 10;
        c++;
    }
    return c;
}


void parse_input(char buffer[BUFLEN], int n, message_udp *new_msg)
{

        new_msg->topic = buffer;

        int i = 50;

        int data_type = (int)buffer[i];

        if(data_type == 0) {
            new_msg->data_type = "INT";
            int sign_byte = (int)buffer[++i]; // i = 51

            int payload_val = ntohl(*(uint32_t *) (buffer + (++i))); // i = 52
            if(sign_byte == 1) {
                payload_val *= (-1);
            }
            char temp[50];
            snprintf(temp, 50, "%d", payload_val);
            new_msg->payload = temp;
        } else if (data_type == 1) {
            new_msg->data_type = "SHORT_REAL";

            float payload_val = ntohs(* (uint16_t *) (buffer + (++i))) / 100.0; // i = 51
            char temp[100];
            snprintf(temp, 100, "%.2f", payload_val);

            new_msg->payload = temp;
        } else if (data_type == 2) {

            new_msg->data_type = "FLOAT";

            int sign_byte = (int)(buffer[++i]); // i = 51

            int payload_val = ntohl(*(uint32_t *) (buffer + (++i))); // i = 52

            uint8_t pow = (*(uint8_t *) (buffer + (i + 4))); // i = 56

            uint8_t pow_cpy = pow;
            int ten_pow = 1;
            while(pow_cpy != 0) {
                ten_pow *= 10;
                pow_cpy--;
            }
            double payload_float_val = payload_val / (ten_pow * 1.0);

            if(sign_byte == 1) {
                payload_float_val *= (-1);
            }
            
            char temp[100];
            snprintf(temp, 100, "%1.10g", payload_float_val);
            new_msg->payload = temp;

        } else if(data_type == 3) {
            new_msg->data_type = "STRING";
            char* payload_val = (char *) (buffer + 51);
            new_msg->payload = payload_val;
        } else {
            printf("\n-----------INVALID !!!!!!!-------\n");
        }
}


bool hasKey(unordered_map<string, int> map, string key) {
    if(map.count(key) == 0) {
        return false;
    }
    return true;
}

bool hasKey(unordered_map<string, queue<message_udp>> map, string key) {
    if(map.size() == 0) {
        return false;
    }

    if(map.count(key) == 0) {
        return false;
    }
    return true;
}

void disconnect_client(int sockfd) {
    char buffer[6];
    
    memset(buffer, 0, 6);
    strcpy(buffer, "close");
    int res = send(sockfd, buffer, strlen(buffer) + 1, 0);
    DIE(res < 0, "res");
    close(sockfd);
}

void send_UDP_client(message_udp curr_msg, int socket) {
    string tot = curr_msg.ip_udp + ":" + curr_msg.port_udp + " - " + curr_msg.topic + " - "
                + curr_msg.data_type + " - " + curr_msg.payload;

    char dim[10];
    memset(dim, 0, 10);
    sprintf(dim, "%ld", tot.size());
    dim[10] = '\0'; 
    send(socket, dim, 10, 0);

    char *msg_char = (char *)malloc((tot.size() + 1) * sizeof(char));
    tot.copy(msg_char, tot.size());
    msg_char[tot.size()] ='\0';

    send(socket, msg_char, tot.size(), 0);
    free(msg_char);
}

int main(int argc, char *argv[]) 
{ 
  
    setvbuf(stdout, NULL, _IONBF, BUFSIZ); 
    char buffer[BUFLEN]; 
    struct sockaddr_in servaddr, cliaddr; 


    // set port
    int portno = atoi(argv[1]);
    DIE(portno == 0, "atoi");
        
    // set server info
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family   = AF_INET; // IPv4 
    servaddr.sin_addr.s_addr = INADDR_ANY; 
    servaddr.sin_port = htons(portno); 


    int sockfd_UDP;
    // UDP file descriptor
    if ( (sockfd_UDP = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
        
    memset(&cliaddr, 0, sizeof(cliaddr)); 

        
    // Bind the socket with the server address 
    int ret = bind(sockfd_UDP, (struct sockaddr *) &servaddr, sizeof(struct sockaddr));
    DIE(ret < 0, "bind");

    int n; 

    socklen_t clilen;

    fd_set read_fds;	// reading set used by select()
    fd_set tmp_fds;		// aux reading set
    int fdmax;			// max fd from read set

    // clear read and aux file descriptors sets
    FD_ZERO(&read_fds);
    FD_ZERO(&tmp_fds);

    // set TCP socket
    int sockfd_TCP, newsockfd;
    sockfd_TCP = socket(AF_INET, SOCK_STREAM, 0);
    DIE(sockfd_TCP < 0, "socket");

    // make link between TCP socket and server
    ret = bind(sockfd_TCP, (struct sockaddr *) &servaddr, sizeof(struct sockaddr));
    DIE(ret < 0, "bind");

    // set TCP socket as a listener for upcoming requests
    ret = listen(sockfd_TCP, MAX_CLIENTS);
    DIE(ret < 0, "listen");


    // add TCP, UDP and STDIN file descriptors to read set
    FD_SET(sockfd_TCP, &read_fds);
    FD_SET(sockfd_UDP, &read_fds);
    FD_SET(STDIN_FILENO, &read_fds);
    fdmax = sockfd_TCP;

    // declare data structures used by the server
    client_tcp clients[MAX_CLIENTS];
    int clients_dim = 0;
    unordered_map<string, queue<message_udp>> inactive_list;
        
    bool running = true;
    while(running) {

        tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
        int i;
		for (i = 0; i <= fdmax; i++) {
			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd_TCP) {
                    // request from a TCP client on the socket listening
					clilen = sizeof(cliaddr);
                    // accept connection
					newsockfd = accept(sockfd_TCP, (struct sockaddr *) &cliaddr, &clilen);
					DIE(newsockfd < 0, "accept");

					// add new socket
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
                    }

                    // empty buffer
                    memset(buffer, 0, BUFLEN);
                    int m = recv(newsockfd, buffer, sizeof(buffer), 0);
                    DIE(m < 0, "recv");

                    // check if the client has a valid id
                    bool valid_new_client = true;
                    for(int j = 0; j < clients_dim; j++) {
                        if(strcmp(clients[j].id, buffer) == 0) {
                            if(clients[j].active) {
                                printf("Client %s already connected.\n", clients[j].id);
                                valid_new_client = false;
                                disconnect_client(newsockfd);
                                FD_CLR(newsockfd, &read_fds);
                            }
                        }
                    }

                    // invalid connecting request with same id
                    if(!valid_new_client) {
                        continue;
                    }
     

                    // check if client is reconnecting
                    for(int j = 0; j < clients_dim; j++) {
                        // the client is reconnecting
                        if(strcmp(clients[j].id, buffer) == 0) {
                            valid_new_client = false;
                            clients[j].active = true;
                            printf("New client %s connected from %s : %d\n",
					        buffer, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

                            // send message received while the client was disconnected
                            if(hasKey(inactive_list, clients[j].id)) {
                                while(!inactive_list.at(clients[j].id).empty()) {
                                    message_udp curr_msg = inactive_list.at(clients[j].id).front();
                                    inactive_list.at(clients[j].id).pop();
                                    send_UDP_client(curr_msg, clients[j].socket);
                                }   
                            }
                            break;
                        }
                    }

                    // new client
                    if(valid_new_client) {
                        client_tcp new_client;
                        strncpy(new_client.id, buffer, strlen(buffer) + 1);
                        new_client.socket = newsockfd;
                        new_client.no_topics = 0;
                        new_client.active = true;
                        // new_client.topics = (char **)malloc(1000 * sizeof(char *));
                        clients[clients_dim++] = new_client;
                        printf("New client %s connected from %s : %d\n",
					        buffer, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

                        // deactivate Nagle
                        int yes = 1;
                        int result = setsockopt(new_client.socket,
                                        IPPROTO_TCP,
                                        TCP_NODELAY,
                                        (char *) &yes, 
                                        sizeof(int));    // 1 - on, 0 - off
                        DIE(result < 0, "Nagle");
                    }
                    					
				} else if(i == sockfd_UDP) {
                    // data on UDP socket
                    memset(buffer, 0, sizeof(buffer));
                    memset(&cliaddr, 0, sizeof(cliaddr));
                    n = recvfrom(sockfd_UDP, (char *)buffer, BUFLEN,  
                                MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                                &clilen);
                    DIE(n < 0, "recvfrom");
                    
                    message_udp new_msg;
                    new_msg.ip_udp = inet_ntoa(cliaddr.sin_addr);
                    char tmp[6];
                    sprintf(tmp, "%d", ntohs(cliaddr.sin_port));
                    new_msg.port_udp = tmp;

                    // get input components in different strings and populate
                    // struct message_udp fields
                    parse_input(buffer, n, &new_msg);

                    // check clients that are subscribed to the topic received
                    // from UDP client
                    for(int j = 0; j < clients_dim; j++) {
                        // check if it is subscribed
                        if(hasKey(clients[j].topics, new_msg.topic)) { 
                            if(clients[j].active) {
                                send_UDP_client(new_msg, clients[j].socket);
                            } else {
                                // enqueue message for inactive clients that
                                // are subscribed to the topic with sf = 1
                                if(hasKey(inactive_list, clients[j].id)) {
                                     if(clients[j].topics.at(new_msg.topic) == 1) {
                                         inactive_list.at(clients[j].id).push(new_msg);
                                     }
                                } else {
                                    if(clients[j].topics.at(new_msg.topic) == 1) {
                                        queue<message_udp> new_queue;
                                        new_queue.push(new_msg);
                                        inactive_list.insert(make_pair(clients[j].id, new_queue));
                                    }
                                }
                            }
                        }
                    }

                } else if (i == STDIN_FILENO) {
                        // input from keyboard
                    	memset(buffer, 0, sizeof(buffer));
			            n = read(0, buffer, sizeof(buffer));
                        buffer[n - 1] = '\0';
			            DIE(n < 0, "read"); 
                        if(strcmp(buffer, "exit") == 0) {
                            printf("Closing server and all conections with clients...\n");
                            for(int j = 0; j < clients_dim; j++) {
                                if(clients[j].active) {
                                    disconnect_client(clients[j].socket);
                                }
                                FD_CLR(clients[j].socket, &read_fds);
                            }
                            close(sockfd_TCP);
                            close(sockfd_UDP);
                            running = false;
                        } else {
                            printf("Invalid command. Command help:\n\"exit:\" closes the server and all conections\n");
                        }
                } else {
					// data on already connected clients socket
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer) - 1, 0);
					DIE(n < 0, "recv");

                    // the client has closed the connection
                    if (n == 0) {
                        for(int j = 0; j < clients_dim; j++) {
                            if(clients[j].socket == i) {
                                printf("Client %s disconnected.\n", clients[j].id);
                                clients[j].active = false;  
                            }
                        }
                        close(i);
						FD_CLR(i, &read_fds);
                        continue;
					}

                    // subscribe / unsubscribe command
                    char topic_subscribed[MAX_TOPIC];
                    int sf;
                    if(buffer[strlen(buffer) - 1] == 'S') {
                        strncpy(topic_subscribed, buffer, strlen(buffer) - 2);
                        topic_subscribed[strlen(buffer) - 2] = '\0';
                        sf = buffer[strlen(buffer) - 2] == '1' ? 1:0;

                        for(int j = 0; j < clients_dim; j++) {
                            if(i == clients[j].socket) {
                                clients[j].topics.insert(make_pair(topic_subscribed, sf));
                            }
                        }
                    } else if (buffer[strlen(buffer) - 1] == 'U') {
                            strncpy(topic_subscribed, buffer, strlen(buffer) - 2); 
                            for(int j = 0; j < clients_dim; j++) {
                                if(i == clients[j].socket) {
                                    clients[j].topics.erase(topic_subscribed);

                                }
                            }
                    }
				}
			}
		}
    }

    close(sockfd_TCP);
    close(sockfd_UDP);
    return 0; 
}