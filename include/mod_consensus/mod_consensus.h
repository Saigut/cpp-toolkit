#ifndef CPP_TOOLKIT_MOD_CONSENSUS_H
#define CPP_TOOLKIT_MOD_CONSENSUS_H

class cppt_consensus {
public:
    int main() {
        while (true) {
            main_step();
        }
    }

    int main_step() {
        msg_recv();
        process_msg();
        maybe_send_msg();
        return 0;
    }

    virtual int process_msg(int p) {
        return -1;
    }

    virtual int maybe_send_msg() {
        return -1;
    }

    int msg_recv() {
        return -1;
    }

    int msg_send_to() {
        return -1;
    }
    int msg_broadcast() {
        return -1;
    }

    int process_msg() {
        return -1;
    }

    int consensus_process() {
        return -1;
    }

    int storage_process() {
        return -1;
    }
};


class cppt_consensus_pow : public cppt_consensus {
public:
    int process_msg(int p) override {
        switch (p) {
            case 1: {
                process_msg_set_data();
                break;
            }
            case 2: {
                process_msg_app_request();
                break;
            }
            default:
                ;
        }
        return 0;
    }

    int maybe_send_msg() override {
        send_msg_set_data();
        send_msg_app_reponse();
        return -1;
    }

    int process_msg_set_data() {
        return -1;
    }
    int process_msg_app_request() {
        return -1;
    }
    int send_msg_set_data() {
        return -1;
    }
    int send_msg_app_reponse() {
        return -1;
    }
};


class cppt_consensus_leader : public cppt_consensus {
public:
    int process_msg(int p) override {
        switch (p) {
            case 1: {
                process_msg_initiate_vote();
                break;
            }
            case 2: {
                process_msg_vote_result();
                break;
            }
            case 3: {
                process_msg_append_data();
                break;
            }
            case 4: {
                process_msg_app_request();
                break;
            }
            default:
                ;
        }
        return 0;
    }

    int maybe_send_msg() override {
        send_msg_initiate_vote();
        send_msg_vote_result();
        send_msg_append_data();
        send_msg_app_reponse();
        return 0;
    }
    int process_msg_initiate_vote() {
        return -1;
    }
    int process_msg_vote_result() {
        return -1;
    }
    int process_msg_append_data() {
        return -1;
    }
    int process_msg_app_request() {
        return -1;
    }
    int send_msg_initiate_vote() {
        return -1;
    }
    int send_msg_vote_result() {
        return -1;
    }
    int send_msg_append_data() {
        return -1;
    }
    int send_msg_app_reponse() {
        return -1;
    }
};


#endif //CPP_TOOLKIT_MOD_CONSENSUS_H
