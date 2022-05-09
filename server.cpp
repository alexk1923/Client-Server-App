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
        int i = 0;
        while(buffer[i] != '\0' && i < 50) {
            i++;
        }

        
        strncpy(new_msg->topic, buffer, i);
        (new_msg->topic)[i] = '\0';
        // printf("Topic: %s", topic);
        i = 50;


        // type = buffer[i];
        // printf("type:%d", (int)type);

        int data_type = (int)buffer[i];     
        // printf("Tip de date: 0x%d ", data_type);

        if(data_type == 0) {
            // printf("\n---------INT!---------\n");

            strncpy(new_msg->data_type, "INT", 3);
            new_msg->data_type[3] = '\0';
            int sign_byte = (int)buffer[++i]; // i = 51
            // printf("Octet de semn %d\n", sign_byte);

            int payload_val = ntohl(*(uint32_t *) (buffer + (++i))); // i = 52
            if(sign_byte == 1) {
                payload_val *= (-1);
            }
            
            sprintf(new_msg->payload, "%d", payload_val);
            new_msg->payload[strlen(new_msg->payload)] = '\0';
            // printf("uint32_t in network: %d\n",  payload_val);
        } else if (data_type == 1) {
            strncpy(new_msg->data_type, "SHORT_REAL", 10);
            new_msg->data_type[10] = '\0';


            // printf("\n---------SHORT REAL!---------\n");
            float payload_val = ntohs(* (uint16_t *) (buffer + (++i))) / 100.0; // i = 51

            sprintf(new_msg->payload, "%.2f", payload_val);
            new_msg->payload[strlen(new_msg->payload)] = '\0';
            // printf("Number = %.2f\n", payload_val);
        } else if (data_type == 2) {
            // printf("\n---------FLOAT!---------\n");
            strncpy(new_msg->data_type, "FLOAT", 5);
            new_msg->data_type[5] = '\0';



            int sign_byte = (int)(buffer[++i]); // i = 51
            // printf("Octet de semn %d\n", sign_byte);

            int payload_val = ntohl(*(uint32_t *) (buffer + (++i))); // i = 52

            uint8_t pow = (*(uint8_t *) (buffer + (i + 4))); // i = 56

            uint8_t pow_cpy = pow;
            int ten_pow = 1;
            while(pow_cpy != 0) {
                ten_pow *= 10;
                pow_cpy--;
            }
            // printf("tenpow:%d", ten_pow);
            double payload_float_val = payload_val / (ten_pow * 1.0);

            if(sign_byte == 1) {
                payload_float_val *= (-1);
            }
            

            // printf("payload float val: %d", (int)payload_float_val);
            // char aux_float[10];
            // sprintf(aux_float, "%d", (int)payload_float_val);
            sprintf(new_msg->payload, "%1.10g", payload_float_val);
            new_msg->payload[strlen(new_msg->payload)] = '\0';
            // printf("float_number:%f\n", payload_float_val);

        } else if(data_type == 3) {

            strncpy(new_msg->data_type, "STRING", 6);
            new_msg->data_type[6] = '\0';
            // printf("\n---------STRING!---------\n");
            char* payload_val = (char *) (buffer + 51);
            strncpy(new_msg->payload, payload_val, strlen(payload_val));
            // printf("strlen payload string: %ld", strlen(payload_val));
            new_msg->payload[strlen(payload_val)] = '\0';
            // printf("payload string:%s\n", payload_val);
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
        FD_SET(sockfd_UDP, &read_fds);
        FD_SET(STDIN_FILENO, &read_fds);
        fdmax = sockfd_TCP;

        client_tcp clients[1000];
        int clients_dim = 0;
        unordered_map<string, queue<message_udp>> inactive_list;
        
    bool running = true;
    while(running) {

    /************** PROTOCOLUL UDP ************/

        // continue;
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


                    memset(buffer, 0, BUFLEN);
                    int m = recv(newsockfd, buffer, sizeof(buffer), 0);
                    DIE(m < 0, "recv");


                    bool valid_new_client = true;
                    for(int j = 0; j < clients_dim; j++) {
                        if(strcmp(clients[j].id, buffer) == 0) {
                            if(clients[j].active) {
                                printf("Client %s already connected.\n", clients[j].id);
                                valid_new_client = false;
                                memset(buffer, 0, BUFLEN);
                                strcpy(buffer, "close");
                                int res = send(newsockfd, buffer, strlen(buffer), 0);
                                close(newsockfd);
                                FD_CLR(newsockfd, &read_fds);
                            }
                        }
                    }

                    // daca s-a incercat conectarea unui client cu acelasi ID
                    if(!valid_new_client) {
                        continue;
                    }
     

                    // cazul in care se RECONECTEAZA un client
                    for(int j = 0; j < clients_dim; j++) {
                        if(strcmp(clients[j].id, buffer) == 0) {
                            // printf("Acest client a mai fost conectat inainte\n");
                            valid_new_client = false;
                            clients[j].active = true;
                            printf("New client %s connected from %s : %d\n",
					        buffer, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
                            
                            // printf("------------------------------------------------");
                            // print_inactive_list(inactive_list);
                            // printf("-----------------------------------------------");
                            if(hasKey(inactive_list, clients[j].id)) {
                                while(!inactive_list.at(clients[j].id).empty()) {
                                    message_udp curr_msg = inactive_list.at(clients[j].id).front();
                                    // printf("Voi trimite mesajul:\n");
                                    // print_udp_msg(curr_msg);
                                    inactive_list.at(clients[j].id).pop();
                                    send(clients[j].socket, &curr_msg, 1600, 0);
                                }   
                            }
                            break;
                        }
                    }

                    if(valid_new_client) {
                        client_tcp new_client;
                        strncpy(new_client.id, buffer, strlen(buffer) + 1);
                        new_client.sf = -1;
                        new_client.socket = newsockfd;
                        new_client.no_topics = 0;
                        new_client.active = true;
                        // new_client.topics = (char **)malloc(1000 * sizeof(char *));
                        clients[clients_dim++] = new_client;
                        printf("New client %s connected from %s : %d\n",
					        buffer, inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));

                        int yes = 1;
                        int result = setsockopt(new_client.socket,
                                        IPPROTO_TCP,
                                        TCP_NODELAY,
                                        (char *) &yes, 
                                        sizeof(int));    // 1 - on, 0 - off
                        // printf("Bufferul primit este: %s", buffer);
                    }
                    					
				} else if(i == sockfd_UDP) {
                    // a venit o cerere de conexiune pe socketul inactiv (cel cu listen),
					// pe care serverul o accepta
                    
                    memset(buffer, 0, sizeof(buffer));
                    memset(&cliaddr, 0, sizeof(cliaddr));
                    n = recvfrom(sockfd_UDP, (char *)buffer, BUFLEN,  
                                MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                                &len);
                    DIE(n < 0, "recvfrom");
                    
                    char topic[51];
                    char type[2];
                    char payload[1501];
                    
                    message_udp new_msg;
                    memset(&new_msg, 0, sizeof(new_msg));

                    sprintf(new_msg.ip_udp, "%s", inet_ntoa(cliaddr.sin_addr));
                    sprintf(new_msg.port_udp, "%d", ntohs(cliaddr.sin_port));
                    // printf("strlen:%ld\n", strlen(new_msg.port_udp));
                    new_msg.port_udp[strlen(new_msg.ip_udp)] = '\0';
                    new_msg.port_udp[strlen(new_msg.port_udp)] = '\0';

                    parse_input(buffer, n, &new_msg);

					// clilen = sizeof(cliaddr);

					// printf("Noua conexiune UDP de la %s, port %d, socket client %d\n",
					// 		inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), newsockfd);

                    // printf("%s:%d", inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port));
                    
                    // print_clients(clients, clients_dim);

                    for(int j = 0; j < clients_dim; j++) {
                        if(hasKey(clients[j].topics, new_msg.topic)) { // daca e abonat
                            // print_udp_msg(new_msg);
   
                            
                            if(clients[j].active) {
                                send(clients[j].socket, &new_msg, 1600, 0);
                            } else {
                                if(hasKey(inactive_list, clients[j].id)) {
                                     if(clients[j].topics.at(new_msg.topic) == 1) {
                                        // printf("------------------------------------------------");
                                        // print_inactive_list(inactive_list);
                                        // printf("-----------------------------------------------");
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
                    	memset(buffer, 0, sizeof(buffer));
			            n = read(0, buffer, sizeof(buffer));
                        // printf("n = %d", n);
                        buffer[n - 1] = '\0';
			            DIE(n < 0, "read"); 
                        if(strcmp(buffer, "exit") == 0) {
                            // TODO: Inchide toate conexiunile active
                            printf("Se inchide serverul si toate conexiunile active de TCP\n");
                            for(int j = 0; j < clients_dim; j++) {
                                if(clients[j].active) {
                                    memset(buffer, 0, BUFLEN);
                                    strcpy(buffer, "close");
                                    int res = send(clients[j].socket, buffer, strlen(buffer), 0);
                                    DIE(res < 0, "res");
                                }
                                close(clients[j].socket);
                                FD_CLR(clients[j].socket, &read_fds);
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
						// printf("Client %d disconnected\n", i);
						// close(i);

                        for(int j = 0; j < clients_dim; j++) {
                            if(clients[j].socket == i) {
                                printf("Client %s disconnected.\n", clients[j].id);
                                clients[j].active = false;  
                            }
                        }
                        
						// se scoate din multimea de citire socketul inchis 
                        close(i);
						FD_CLR(i, &read_fds);
                        continue;
					}


                    int no_arg = 0;

                    char topic_subscribed[MAX_TOPIC];
                    int sf;
                    if(buffer[strlen(buffer) - 1] == 'S') {
                        strncpy(topic_subscribed, buffer, strlen(buffer) - 2);
                        topic_subscribed[strlen(buffer) - 2] = '\0';
                        sf = buffer[strlen(buffer) - 2] == '1' ? 1:0;

                        for(int j = 0; j < clients_dim; j++) {
                            // printf("i = %d, client[%d] = %d", i, j, clients[j].socket);
                            if(i == clients[j].socket) {
                                // printf("%s Subscribed to %s\n", clients[j].id, topic_subscribed);
                                // printf("Clientul a fost gasit!\n");
                                clients[j].topics.insert(make_pair(topic_subscribed, sf));
                                // send(clients[j].socket, msg, strlen(msg), 0);
                                // print_clients(clients, clients_dim);
                                // clients[j].topic[clients[j].no_topics] = (char *)malloc(MAX_TOPIC * sizeof(char));
                                // strncpy(clients[j].topic[clients[j].no_topics++], topic_subscribed,
                                //         strlen(topic_subscribed) + 1);
                            }
                        }
                    } else if (buffer[strlen(buffer) - 1] == 'U') {
                        // printf("------------------------");
                            strncpy(topic_subscribed, buffer, strlen(buffer) - 2); // aici e topic unsubscribed
                            // printf("topic_unsubscribed = %s", topic_subscribed);
                        
                        if(no_arg == 2) {
                            for(int j = 0; j < clients_dim; j++) {
                                if(i == clients[j].socket) {
                                    // printf("%s unsubscribed from %s\n", clients[j].id, topic_subscribed);
                                    // printf("topic care va fi erase-uit:%s|\n", topic_subscribed);
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