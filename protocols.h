#ifndef  HASHTABLE_H_
#define  HASHTABLE_H_

#include <iostream>
#include<vector>
using namespace std;
#include <unordered_map>
#include <queue>

#define MAX_TOPIC 51
#define MAX_ID 150
#define MAX_MSG_LIST 200
#define MAX_STORED_CLIENTS 1000

#pragma pack(1)
typedef struct message_udp
{
    int dim;
    string ip_udp;
    string port_udp;
    string topic;
    string data_type;
    string payload;

}message_udp;
#pragma pack(0)

typedef struct client_tcp
{
    char id[MAX_ID];
    int socket;
    int no_topics;
    unordered_map<string, int> topics;
    bool active;

}client_tcp;


void print_udp_msg(message_udp udp_msg);
void print_clients(client_tcp clients[], int client_dim);
void print_inactive_list(unordered_map<string, queue<message_udp>> map);


#endif