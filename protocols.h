#ifndef  HASHTABLE_H_
#define  HASHTABLE_H_

#include <iostream>
#include<vector>
using namespace std;
#include <unordered_map>

#define MAX_TOPIC 51
#define MAX_ID 150

typedef struct message_udp
{
    char ip_udp[16];
    char port_udp[6];
    char topic[MAX_TOPIC];
    char data_type[11];
    char payload[1501];

}message_udp;

typedef struct client_tcp
{
    char id[MAX_ID];
    int socket;
    int no_topics;
    unordered_map<string, int> topics;
    int sf;
    bool active;

}client_tcp;

void print_udp_msg(message_udp udp_msg);
void print_clients(client_tcp clients[], int client_dim);

#endif