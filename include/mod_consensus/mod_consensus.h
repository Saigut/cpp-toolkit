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
        recv_msg();
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

    int msg_recv();
    int msg_send_to();
    int msg_broadcast();

    int consensus_process();

    int storage_process();
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
    }

    int maybe_send_msg() override {
        send_msg_set_data();
        send_msg_app_reponse();
    }
};


class cppt_consensus_leader {
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
    }

    int maybe_send_msg() override {
        send_msg_initiate_vote();
        send_msg_vote_result();
        send_msg_append_data();
        send_msg_app_reponse();
    }
};


#endif //CPP_TOOLKIT_MOD_CONSENSUS_H
