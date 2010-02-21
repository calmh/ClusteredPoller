#include "multithread.h"
#include <iostream>
using namespace std;

Multithread::Multithread(int num_threads)
	: num_threads(num_threads), next_thread_id(0)
{
	threads = new pthread_t[num_threads];
}

Multithread::~Multithread()
{
	delete[] threads;
}

void Multithread::start()
{
	for (int i = 0; i < num_threads; i++) {
		int thread_id = next_thread_id++;
		cerr << "Start thread, id " << thread_id << endl;
    create_thread(&threads[i], thread_id);
	}
}

void Multithread::join_all()
{
	for (int i = 0; i < num_threads; i++) {
		pthread_join(threads[i], NULL);
		cerr << "Joined thread " << i << endl;
	}
}
