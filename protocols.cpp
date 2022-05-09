#include "protocols.h"


void print_udp_msg(message_udp udp_msg) {
    printf("++++++++\n");
    printf("IP UDP:%s\n", udp_msg.ip_udp);
    printf("Port UDP:%s\n", udp_msg.port_udp);
    printf("Topic:%s\n", udp_msg.topic);
    printf("Type:%s\n", udp_msg.data_type);
    printf("Payload:%s\n", udp_msg.payload);
    printf("++++++++\n");
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

