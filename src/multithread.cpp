#include "multithread.h"

#include <iostream>
using namespace std;

Multithread::Multithread(int num_threads)
                : num_threads(num_threads)
{
        threads = new pthread_t[num_threads];
        thread_ids = new int[num_threads];
}

Multithread::~Multithread()
{
        delete[] threads;
        delete[] thread_ids;
}

void Multithread::start()
{
        for (int i = 0; i < num_threads; i++) {
                thread_ids[i] = i;
                create_thread(&threads[i], &thread_ids[i]);
        }
}

void Multithread::join_all()
{
        for (int i = 0; i < num_threads; i++) {
                pthread_join(threads[i], NULL);
        }
}
