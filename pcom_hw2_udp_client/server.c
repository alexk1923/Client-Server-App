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

void usage(char *file)
{
	fprintf(stderr, "Usage: %s server_port\n", file);
	exit(0);
}

int main(int argc, char *argv[]) 
{ 
    int sockfd_UDP; 
    char buffer[BUFLEN]; 
    char *hello = "Hello from server"; 
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
            
        int len, n; 
        
        len = sizeof(cliaddr);  //len is value/result 
    
    while(1) {

    /************** PROTOCOLUL UDP ************/
        n = recvfrom(sockfd_UDP, (char *)buffer, BUFLEN,  
                    MSG_WAITALL, ( struct sockaddr *) &cliaddr, 
                    &len); 
        printf("s-au citit %d bytes", n);
        // buffer[50] = '\0';
        int i = 0;
        while(buffer[i] != '\0' && i < 50) {
            printf("%c ", buffer[i]);
            i++;
        }

        printf("i = %d\n", i);

        for(int j = i + 1; j <= 100; j++) {
            if(j == 50) {
                printf("j = 50: ");
            }
             printf("0x%02x ", buffer[j]);
        }

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
        // printf("Client : %d\n",  *(int *) (buffer + 51)); 
        // sendto(sockfd, (const char *)hello, strlen(hello),  
        //     MSG_CONFIRM, (const struct sockaddr *) &cliaddr, 
        //         len); 
        // printf("Hello message sent.\n");  
    }
        
    return 0; 
}