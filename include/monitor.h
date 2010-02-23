#ifndef MONITOR_H_
#define MONITOR_H_

#include "multithread.h"

class Monitor : public Multithread {
protected:
        void create_thread(pthread_t* thread, int* thread_id);
        static void* run(void* id_ptr);

public:
        Monitor();
};

#endif /* MONITOR_H_ */
