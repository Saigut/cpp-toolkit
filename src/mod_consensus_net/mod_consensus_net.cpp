#include <mod_consensus_net/mod_consensus_net.h>

class cppt_consensus_net {
public:
    int main() {
        join_p2p_net();
        run_consensus();

        return 0;
    }
};