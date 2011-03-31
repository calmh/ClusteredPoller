#ifndef POLLER_H_
#define POLLER_H_

#include "multithread.h"

class RTGConf;
class RTGTargets;

class Poller : public Multithread
{
protected:
        void create_thread(pthread_t* thread, int* thread_id);
        static void* run(void* id_ptr);
        static int stride;
        static RTGTargets* hosts;

public:
        Poller(int num_threads, RTGTargets* hosts);
};

#endif /* POLLER_H_ */
