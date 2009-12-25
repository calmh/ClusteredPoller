#include "types.h"
#include "query.h"
#include "version.h"

using namespace std;

// Global variables.
vector<QueryHost> hosts;
vector<ResultCache> cache;
RTGConf config;
unsigned thread_id = 0;

// Command line flags
string rtgconf = "/usr/local/rtg/etc/rtg.conf";
string targets = "/usr/local/rtg/etc/targets.cfg";
int verbosity = 0;
int detach = 1;
int use_db = 1;
int allow_db_zero = 0;

// Locking and statistics
pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
unsigned active_threads = 0;
unsigned stat_inserts = 0;
unsigned stat_queries = 0;
unsigned stat_iterations = 0;

void daemonize(void);
string no_semi(string token);
void thread_loop();
void* start_thread(void *ptr);
RTGConf read_rtg_conf(string filename);
vector<QueryHost> read_rtg_targets(string filename);
void help();
template <typename Iter> void range_tolower (Iter beg, Iter end);
void string_tolower (std::string & str);

/*
 * Setup and initialization.
 */

void help() {
	cerr << "clpoll version " << CLPOLL_VERSION << endl;
	cerr << " -c <file>   Specify configuration file [" << rtgconf << "]" << endl;
	cerr << " -D          Don't detach, run in foreground" << endl;
	cerr << " -d          Disable database inserts" << endl;
	cerr << " -t <file>   Specify target file [" << targets << "]" << endl;
	cerr << " -v          Increase verbosity" << endl;
	cerr << " -z          Database zero delta inserts" << endl;
	cerr <<  " Copyright (c) 2009-2010 Jakob Borg" << endl;
}

int main (int argc, char * const argv[])
{
	if (argc < 2) {
		help();
		exit(0);
	}
	for (int i = 1; i < argc; i++) {
		string arg = string(argv[i]);
		if (arg == "-v")
			verbosity++;
		else if (arg == "-D")
			detach = 0;
		else if (arg == "-d")
			use_db = 0;
		else if (arg == "-z")
			allow_db_zero = 1;
		else if (arg == "-h") {
			help();
			exit(0);
		}
		else if (arg == "-c") {
			i++;
			rtgconf = string(argv[i]);
		}
		else if (arg == "-t") {
			i++;
			targets = string(argv[i]);
		}
	}

	if (detach)
		daemonize();

	init_snmp("clpoll");
	config = read_rtg_conf(rtgconf);
	hosts = read_rtg_targets(targets);

	if (hosts.size() == 0) {
		cerr << "No hosts, so nothing to do." << endl;
		exit(-1);
	}

	cache = vector<ResultCache>(hosts.size());
	pthread_t threads[config.threads];

	if (verbosity >= 1) {
		cerr << "Polling every " << config.interval << " seconds." << endl;
		cerr << "Starting poll with " << config.threads << " threads." << endl;
	}

	for (unsigned i = 0; i < config.threads; i++) {
		pthread_create(&threads[i], NULL, start_thread, NULL);
	}

	for (unsigned i = 0; i < config.threads; i++) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}

/*
 * Threading
 */

void thread_loop()
{
	unsigned stride = config.threads;
	pthread_mutex_lock(&global_lock);
	unsigned offset = thread_id++;
	pthread_mutex_unlock(&global_lock);
	unsigned iterations = 0;
	while (1) {
		// Mark ourself active
		pthread_mutex_lock(&global_lock);
		active_threads++;
		pthread_mutex_unlock(&global_lock);

		time_t start = time(NULL);
		for (unsigned i = offset; i < hosts.size(); i += stride) {
			QueryHost host = hosts[i];
			process_host(host, cache[i]);
		}
		sleep(1);
		time_t end = time(NULL);
		time_t sleep_time = config.interval - (end - start);
		pthread_mutex_lock(&global_lock);

		// Mark ourself sleeping
		active_threads--;
		if (verbosity >= 1) {
			if (iterations < stat_iterations) {
				cerr << "Thread " << offset << " is behind schedule!" << endl;
				cerr << "  My iteration counter: " << iterations << endl;
				cerr << "  Global iteration counter: " << stat_iterations << endl;
			}
			if (active_threads == 0) {
				stat_iterations++;
				cerr << "Iteration " << stat_iterations << " completed." << endl;
				cerr << "  Rows inserted: " << stat_inserts << endl;
				cerr << "  Queries executed: " << stat_queries << endl;
				stat_inserts = 0;
				stat_queries = 0;
			}
		}
		pthread_mutex_unlock(&global_lock);

		iterations++;
		sleep(sleep_time);
	}
}

void* start_thread(void *ptr)
{
	thread_loop();
	return NULL;
}

/*
 * Configuration reading.
 */

RTGConf read_rtg_conf(string filename)
{
	ifstream rtgconf(filename.c_str());
	string token;
	RTGConf conf;
	while (rtgconf >> token) {
		string_tolower(token);
		if (token == "interval")
			rtgconf >> conf.interval;
		else if (token == "highskewslop")
			rtgconf >> conf.high_skew_slop;
		else if (token == "lowskewslop")
			rtgconf >> conf.low_skew_slop;
		else if (token == "db_host")
			rtgconf >> conf.dbhost;
		else if (token == "db_database")
			rtgconf >> conf.database;
		else if (token == "db_user")
			rtgconf >> conf.dbuser;
		else if (token == "db_pass")
			rtgconf >> conf.dbpass;
		else if (token == "threads")
			rtgconf >> conf.threads;
	}
	return conf;
}

vector<QueryHost> read_rtg_targets(string filename)
{
	vector<QueryHost> hosts;
	ifstream targets(filename.c_str());
	string token;
	QueryHost host;
	QueryRow row;
	int state = 0;
	int nhosts = 0;
	int ntargs = 0;
	while (targets >> token) {
		token = no_semi(token);
		string_tolower(token);
		if (state == 0) {
			if (token == "host")
				targets >> host.host;
			else if (token == "{")
				state = 1;
		}
		else if (state == 1) {
			if (token == "community") {
				targets >> token;
				host.community = no_semi(token);
			}
			else if (token == "snmpver") {
				targets >> host.snmpver;
			}
			else if (token == "target") {
				targets >> token;
				row.oid = no_semi(token);
			}
			else if (token == "{") {
				state = 2;
			}
			else if (token == "}") {
				hosts.push_back(host);
				nhosts++;
				host = QueryHost();
				state = 0;
			}
		}
		else if (state == 2) {
			if (token == "bits")
				targets >> row.bits;
			else if (token == "table") {
				targets >> token;
				row.table = no_semi(token);
			}
			else if (token == "id") {
				targets >> row.id;
			}
			else if (token == "speed") {
				targets >> row.speed;
			}
			else if (token == "}") {
				host.rows.push_back(row);
				ntargs++;
				row = QueryRow();
				state = 1;
			}
		}
	}

	targets.close();
	cout << "Read " << ntargs << " targets in " << nhosts << " hosts." << endl;
	return hosts;
}

/*
 * Utility functions
 */

void daemonize(void)
{
	pid_t pid, sid;

	if ( getppid() == 1 ) return;

	pid = fork();
	if (pid < 0) {
		exit(EXIT_FAILURE);
	}

	if (pid > 0) {
		exit(EXIT_SUCCESS);
	}

	umask(0);

	sid = setsid();
	if (sid < 0) {
		exit(EXIT_FAILURE);
	}

	if ((chdir("/")) < 0) {
		exit(EXIT_FAILURE);
	}

	freopen( "/dev/null", "r", stdin);
	freopen( "/dev/null", "w", stdout);
	freopen( "/dev/null", "w", stderr);
}

string no_semi(string token)
{
	size_t semicolon = token.find(';', 0);
	if (semicolon != string::npos)
		token = token.erase(semicolon, 1);
	return token;
}

template <typename Iter> void range_tolower (Iter beg, Iter end)
{
	for( Iter iter = beg; iter != end; ++iter ) {
		*iter = std::tolower( *iter );
	}
}

void string_tolower (std::string & str)
{
	range_tolower(str.begin(), str.end());
}
