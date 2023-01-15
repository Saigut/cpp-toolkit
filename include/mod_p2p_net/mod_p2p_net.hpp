#ifndef CPP_TOOLKIT_MOD_P2P_NET_H
#define CPP_TOOLKIT_MOD_P2P_NET_H

#include <stdint.h>
#include <stddef.h>

class cppt_p2p_node {
public:
    int node_maintain_nodes() {
//        node_add_init_nodes();
//        node_connect_nodes();
//        node_remove_unconnectabled_node();
        return 0;
    }
    int node_find(uint8_t* node_id, size_t node_id_len) {
        return -1;
    }
    int node_add(uint8_t* node_id, size_t node_id_len) {
        return -1;
    }

    int msg_process_income_msgs() {
//        node_add();
        return 0;
    }
    int msg_send_to_node(uint8_t* node_id, size_t node_id_len,
                         uint8_t* msg, size_t msg_len) {
//        node_find();
//        node_connect();
        return -1;
    }
    int msg_broadcast(uint8_t* node_id, size_t node_id_len,
                      uint8_t* msg, size_t msg_len) {
        return -1;
    }
};

#endif //CPP_TOOLKIT_MOD_P2P_NET_H
