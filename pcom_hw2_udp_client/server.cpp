#include <iostream>
#include<vector>
using namespace std;
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
#include <sys/time.h>


#define MAX_TOPIC 50
#define MAX_ID 150

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

typedef struct client_tcp
{
    char id[MAX_ID];
    int socket;
    int no_topics;
    unordered_map<string, int> topics;
    int sf;

}client_tcp;

void parse_input(char buffer[BUFLEN], int n)
{
        printf("S-au citit %d bytes", n);
        int i = 0;
        while(buffer[i] != '\0' && i < 50) {
            printf("%c ", buffer[i]);
            i++;
        }

        printf("i = %d\n", i);

        // for(int j = i + 1; j <= 100; j++) {
        //     if(j == 50) {
        //         printf("j = 50: ");
        //     }
        //      printf("0x%02x ", buffer[j]);
        // }

        i = 50;
        int data_type = (int)buffer[i];     
        printf("Tip de date: 0x%d ", data_type);

        if(data_type == 0) {
            printf("\n---------INT!---------\n");

            int sign_byte = (int)buffer[++i]; // i = 51
            printf("Octet de semn %d\n", sign_byte);

            int payload = ntohl(*(uint32_t *) (buffer + (++i))); // i = 52
            if(sign_byte == 1) {
                payload *= (-1);
            }

            printf("uint32_t in network: %d\n",  payload);
        } else if (data_type == 1) {
            printf("\n---------SHORT REAL!---------\n");
            float payload = ntohs(* (uint16_t *) (buffer + (++i))) / 100.0; // i = 51      
            printf("Number = %.2f\n", payload);
        } else if (data_type == 2) {
            printf("\n---------FLOAT!---------\n");
            int sign_byte = (int)(buffer[++i]); // i = 51
            printf("Octet de semn %d\n", sign_byte);

            int payload = ntohl(*(uint32_t *) (buffer + (++i))); // i = 52

            uint8_t pow = (*(uint8_t *) (buffer + (i + 4))); // i = 56

            int ten_pow = 1;
            while(pow != 0) {
                ten_pow *= 10;
                pow--;
            }
            printf("tenpow:%d", ten_pow);
            double float_number = payload / (ten_pow * 1.0);

            if(sign_byte == 1) {
                float_number *= (-1);
            }

            printf("float_number:%lf\n", float_number);

        } else if(data_type == 3) {
            printf("\n---------STRING!---------\n");
            char *payload = (char *) (buffer + 51);
            printf("payload string:%s\n", payload);
        } else {
            printf("\n-----------INVALID !!!!!!!-------\n");
        }

        printf("\n");
}

void print_clients(client_tcp clients[], int client_dim) {
    for(int i = 0; i < client_dim; i++) {
        printf("Client:\n");
        printf("Id: %s\n", clients[i].id);
        printf("Socket: %d\n", clients[i].socket);
        printf("No topics:%d\n", clients[i].no_topics);
        // for(int j = 0; j < clients[i].no_topics; j++) {
        //     printf("Topic: %s\n", clients[i].topic[j]);
        // }

        vector<char *> keys((clients[i].topics).size());
        vector<int> vals((clients[i].topics).size());

        cout << "Contents of the unordered_map : \n";
        for (auto p : clients[i].topics) {
            cout << "[" << (p.first) << ", "
                    << (p.second) << "]\n";
        }
    }

}

int main(int argc, char *argv[]) 
{ 
    setvbuf(stdout, NULL, _IONBF, BUFSIZ);
    int sockfd_UDP; 
    char buffer[BUFLEN]; 
    // char *hello = "Hello from server"; 
    struct sockaddr_in servaddr, cliaddr; 

        // Creating socket file descriptor 
        if ( (sockfd_UDP = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
            perror("socket creation failed"); 
            exit(EXIT_FAILURE); 
        } 
            
         
        memset(&cliaddr, 0, sizeof(cliaddr)); 

        // setare port
        int portno = atoi(argv[1]);
        DIE(portno == 0, "atoi");
            
        // Informatii despre server
        memset(&servaddr, 0, sizeof(servaddr));
        servaddr.sin_family   = AF_INET; // IPv4 
        servaddr.sin_addr.s_addr = INADDR_ANY; 
        servaddr.sin_port = htons(portno); 
            
        // Bind the socket with the server address 
        int ret = bind(sockfd_UDP, (struct sockaddr *) &servaddr, sizeof(struct sockaddr));
	    DIE(ret < 0, "bind");
            
        socklen_t len;
        int n; 
        
        len = sizeof(cliaddr);  //len is value/result 

        socklen_t clilen;

        fd_set read_fds;	// multimea de citire folosita in select()
        fd_set tmp_fds;		// multime folosita temporar
        int fdmax;			// valoare maxima fd din multimea read_fds

        // se goleste multimea de descriptori de citire (read_fds) si multimea temporara (tmp_fds)
        FD_ZERO(&read_fds);
        FD_ZERO(&tmp_fds);

        int sockfd_TCP, newsockfd;
        sockfd_TCP = socket(AF_INET, SOCK_STREAM, 0);
	    DIE(sockfd_TCP < 0, "socket");
    


        ret = bind(sockfd_TCP, (struct sockaddr *) &servaddr, sizeof(struct sockaddr));
        DIE(ret < 0, "bind");

        ret = listen(sockfd_TCP, MAX_CLIENTS);
        DIE(ret < 0, "listen");

        // se adauga noul file descriptor (socketul pe care se asculta conexiuni) in multimea read_fds
        FD_SET(sockfd_TCP, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        fdmax = sockfd_TCP;

        client_tcp clients[1000];
        int clients_dim = 0;
    
    bool running = true;
    while(running) {

    /************** PROTOCOLUL UDP ************/
        // n = recvfrom(sockfd_UDP, (char *)buffer, BUFLEN,  
        //             MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
        //             &len);

        // printf("-------------------------\n");
        // print_clients(clients, clients_dim);
        // printf("-------------------------\n");

        /************** PROTOCOLUL TCP ************/ 

        tmp_fds = read_fds; 
		
		ret = select(fdmax + 1, &tmp_fds, NULL, NULL, NULL);
		DIE(ret < 0, "select");
        int i;
		for (i = 0; i <= fdmax; i++) {

			if (FD_ISSET(i, &tmp_fds)) {
				if (i == sockfd_TCP) {
                    // printf("cazul 1\n");
					// a venit o cerere de conexiune pe socketul inactiv (cel cu listen) de TCP,
					// pe care serverul o accepta
					clilen = sizeof(cliaddr);
					newsockfd = accept(sockfd_TCP, (struct sockaddr *) &cliaddr, &clilen);
					DIE(newsockfd < 0, "accept");

					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
                    }
					// for(int j = 0; j <= fdmax; j++) {
					// 	if(j != newsockfd && j!=sockfd_TCP && FD_ISSET(j, &read_fds)) {
					// 		char msg[1024];
					// 		sprintf(msg, "Trimitem mesaj catre ceilalti clienti: Clientul %d a fost adaugat", newsockfd);
					// 		// printf(" j = %d", j);
					// 		send(j, msg, strlen(msg), 0);
					// 	}
					// }

                    memset(buffer, 0, BUFLEN);
                    int m = recv(newsockfd, buffer, sizeof(buffer), 0);
                    DIE(m < 0, "recv");

                    printf("New client %s connected from %s : %d\n",
					buffer, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

                    bool old_client = false;
                    for(int j = 0; j < clients_dim; j++) {
                        if(clients[j].socket == newsockfd) {
                            printf("Acest client a mai fost conectat inainte\n");
                            old_client = true;
                            break;
                        }
                    }

                    if(old_client == false) {
                        client_tcp new_client;
                        strncpy(new_client.id, buffer, strlen(buffer) + 1);
                        new_client.sf = -1;
                        new_client.socket = newsockfd;
                        new_client.no_topics = 0;
                        // new_client.topics = (char **)malloc(1000 * sizeof(char *));
                        clients[clients_dim++] = new_client;
                        // printf("Bufferul primit este: %s", buffer);
                    }
                    					
				} else if(i == sockfd_UDP) {
                    // a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
					clilen = sizeof(cliaddr);
					newsockfd = accept(sockfd_TCP, (struct sockaddr *) &cliaddr, &clilen);
					DIE(newsockfd < 0, "accept");

					// se adauga noul socket intors de accept() la multimea descriptorilor de citire
					FD_SET(newsockfd, &read_fds);
					if (newsockfd > fdmax) { 
						fdmax = newsockfd;
					}

					printf("Noua conexiune UDP de la %s, port %d, socket client %d\n",
							inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), newsockfd);

					// for(int j = 0; j <= fdmax; j++) {
					// 	if(j != newsockfd && j!=sockfd_UDP && FD_ISSET(j, &read_fds)) {
					// 		char msg[1024];
					// 		sprintf(msg, "Trimitem mesaj catre ceilalti clienti: Clientul %d a fost adaugat", newsockfd);
					// 		// printf(" j = %d", j);
					// 		send(j, msg, strlen(msg), 0);
					// 	}
					// }


                } else if (i == STDIN_FILENO) {
                    	memset(buffer, 0, sizeof(buffer));
			            n = read(0, buffer, sizeof(buffer));
                        // printf("n = %d", n);
                        buffer[n - 1] = '\0';
			            DIE(n < 0, "read"); 
                        if(strcmp(buffer, "exit") == 0) {
                            // TODO: Inchide toate conexiunile active
                            printf("Se inchide serverul si toate conexiunile active de TCP\n");
                            for(int j = 0; j < clients_dim; j++) {
                                close(clients[j].socket);
                            }
                            close(sockfd_TCP);
                            close(sockfd_UDP);
                            running = false;
                        } else {
                            printf("Invalid command. Command help:\n\"exit:\" closes the server and all conections\n");
                        }
                } else {
					// s-au primit date pe unul din socketii de client,
					// asa ca serverul trebuie sa le receptioneze
					memset(buffer, 0, BUFLEN);
					n = recv(i, buffer, sizeof(buffer) - 1, 0);
					DIE(n < 0, "recv");


                    if (n == 0) {
						// conexiunea s-a inchis
                        // TODO de adaugat numele id in loc de "i"
						printf("Client %d disconnected\n", i);
						close(i);

					// for(int j = 0; j <= fdmax; j++) {
					// 	if(j != i && j != sockfd_TCP && FD_ISSET(j, &read_fds)) {
                        char msg[1024];
                        sprintf(msg, "Clientul %d a inchis conexiunea", i);
							// send(j, msg, strlen(msg), 0);
					// 	}
					// }
						
						// se scoate din multimea de citire socketul inchis 
						FD_CLR(i, &read_fds);
                        continue;
					}

                    char *token;

                    int no_arg = 0;
                    token = strtok(buffer, " ");
                    // printf("token =%s|\n", token);

                    char topic_subscribed[MAX_TOPIC];
                    int sf;
                    if(strcmp(token, "subscribe") == 0) {
                        while(token) {
                            no_arg++;
                            token = strtok(NULL, " ");
                            if(token == NULL && no_arg < 3) {
                                printf("Invalid number of arguments\n");
                                break;																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																														
                            }
                            
                            if(no_arg == 1) {
                                // printf("topic = %s\n", token);
                                strncpy(topic_subscribed, token, strlen(token) + 1);
                            } else if(no_arg == 2) {
                                // printf("sf = %s", token);
                                sf = atoi(token);   
                            }
                        }
                        if(no_arg == 3) {
                            for(int j = 0; j < clients_dim; j++) {
                                if(i == clients[j].socket) {
                                    printf("%s Subscribed to %s\n", clients[j].id, topic_subscribed);
                                    // printf("Clientul a fost gasit!\n");
                                    
                                    // clients[j].topics[topic_subscribed] = sf;
                                    clients[j].topics.insert(make_pair(topic_subscribed, sf));
                                    // print_clients(clients, clients_dim);
                                    // clients[j].topic[clients[j].no_topics] = (char *)malloc(MAX_TOPIC * sizeof(char));
                                    // strncpy(clients[j].topic[clients[j].no_topics++], topic_subscribed,
                                    //         strlen(topic_subscribed) + 1);
                                }
                            }
                        }
                    } else if (strcmp(token, "unsubscribe") == 0) {
                        printf("------------------------");
                        while(token) {
                            no_arg++;
                            token = strtok(NULL, " \n");
                            if(token == NULL && no_arg < 2) {
                                printf("Invalid number of arguments\n");																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																																															
                            }
                            
                            if(no_arg == 1) {
                                printf("topic = %s\n", token);
                                strncpy(topic_subscribed, token, strlen(token) + 1); // aici e topic unsubscribed
                            }
                        }
                        if(no_arg == 2) {
                            for(int j = 0; j < clients_dim; j++) {
                                if(i == clients[j].socket) {
                                    printf("%s unsubscribed from %s\n", clients[j].id, topic_subscribed);
                                    printf("topic care va fi erase-uit:%s|\n", topic_subscribed);
                                    clients[j].topics.erase(topic_subscribed);
                                    // clients[j].topics[topic_subscribed] = sf;
                                    // print_clients(clients, clients_dim);
                                    // clients[j].topic[clients[j].no_topics] = (char *)malloc(MAX_TOPIC * sizeof(char));
                                    // strncpy(clients[j].topic[clients[j].no_topics++], topic_subscribed,
                                    //         strlen(topic_subscribed) + 1);
                                }
                            }
                        } 
                    }
				}
			}
		}


        // printf("Client : %d\n",  *(int *) (buffer + 51)); 
        // sendto(sockfd, (const char *)hello, strlen(hello),  
        //     MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
        //         len); 
        // printf("Hello message sent.\n");  
    }
    close(sockfd_TCP);
    close(sockfd_UDP);

    return 0; 
}