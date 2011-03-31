#ifndef POLLER_H_
#define POLLER_H_

#include "multithread.h"

class RTGConf;
class RTGTargets;
class ResultCache;

class Poller : public Multithread
{
protected:
        void create_thread(pthread_t* thread, int* thread_id);
        static void* run(void* id_ptr);
        static int stride;
        static RTGTargets* hosts;
        static std::vector<ResultCache> *cache;

public:
        Poller(int num_threads, RTGTargets* hosts);
};

#endif /* POLLER_H_ */
