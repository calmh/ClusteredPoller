#ifndef DATABASE_H_
#define DATABASE_H_

#include "multithread.h"

class Database : public Multithread {
private:
	static std::string dequeue_query();

protected:
	void create_thread(pthread_t* thread, int thread_id);
	static void* run(void* id_ptr);

public:
	Database(int num_threads);
};

#endif /* DATABASE_H_ */
