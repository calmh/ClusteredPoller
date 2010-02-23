#ifndef MULTITHREAD_H_
#define MULTITHREAD_H_

#include <pthread.h>

class Multithread
{
private:
        int num_threads;
        pthread_t *threads;
        int *thread_ids;

protected:
        virtual void create_thread(pthread_t* thread, int* thread_id) = 0;

public:
        Multithread(int num_threads);
        ~Multithread();
        void start();
        void join_all();
};

#endif /* MULTITHREAD_H_ */
