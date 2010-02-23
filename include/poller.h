#ifndef POLLER_H_
#define POLLER_H_

#include "multithread.h"

class Poller : public Multithread {
protected:
        void create_thread(pthread_t* thread, int* thread_id);
        static void* run(void* id_ptr);

public:
        Poller(int num_threads);
};

#endif /* POLLER_H_ */
