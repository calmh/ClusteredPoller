#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <list>
#include <map>
#include <vector>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>
#include <pthread.h>
#ifdef MYSQL
#include <mysql++.h>
#endif

#define INTERVAL 30
#define THREADS 16
#define DB "rtg"
#define SERVER "localhost"
#define USER "rtg"
#define PASS "password"

typedef unsigned long long counter_t;

struct ResultCache {
	std::map<std::pair<std::string, int>, counter_t> counters;
	std::map<std::pair<std::string, int>, time_t> times;
};

struct ResultRow {
	int id;
	counter_t counter;
	counter_t rate;
	unsigned bits;

	ResultRow(int iid, counter_t icounter, counter_t irate, unsigned ibits) {
		id = iid;
		counter = icounter;
		rate = irate;
		bits = ibits;
	}
};

struct ResultSet {
	std::string table;
	std::list<ResultRow> rows;

	ResultSet() {}
	ResultSet(std::string itable) {
		table = itable;
	}
};

struct QueryRow {
	std::string oid;
	std::string table;
	int id;
	int bits;

	QueryRow() {}
	QueryRow(std::string ioid, std::string itable, int iid, int ibits) {
		oid = ioid;
		table = itable;
		id = iid;
		bits = ibits;
	}
};

struct QueryHost {
	std::string host;
	std::string community;
	int snmpver;
	std::list<QueryRow> rows;

	QueryHost() {
		host = "none";
	}
	QueryHost(std::string ihost, std::string icommunity, int isnmpver) {
		host = ihost;
		community = icommunity;
		snmpver = isnmpver;
	}
};

std::vector<QueryHost> hosts;
std::vector<ResultCache> cache;

std::map<std::string, ResultSet> query(QueryHost qh)
{
	std::map<std::string, ResultSet> rs;

	struct snmp_session session, *ss;
	struct snmp_pdu *pdu;
	struct snmp_pdu *response;
	oid anOID[MAX_OID_LEN];
	size_t anOID_len = MAX_OID_LEN;
	struct variable_list *vars;
	int status;
	void *sessp;

	snmp_sess_init(&session);
	session.peername = (char*) qh.host.c_str();
	session.version = SNMP_VERSION_2c;
	session.community = (u_char*) qh.community.c_str();
	session.community_len = qh.community.length();
	sessp = snmp_sess_open(&session);

	if (!sessp) {
		snmp_perror("ack");
		snmp_log(LOG_ERR, "something horrible happened!!!\n");
		return rs;
	}

	ss = snmp_sess_session(sessp);

	std::list<QueryRow>::iterator it;
	for (it = qh.rows.begin(); it != qh.rows.end(); it++) {
		QueryRow row = *it;
		if (rs.find(row.table) == rs.end()) {
			ResultSet r(row.table);
			rs[row.table] = r;
		}
		pdu = snmp_pdu_create(SNMP_MSG_GET);
		read_objid(row.oid.c_str(), anOID, &anOID_len);
		snmp_add_null_var(pdu, anOID, anOID_len);

		status = snmp_sess_synch_response(sessp, pdu, &response);

		if (status == STAT_SUCCESS && response->errstat == SNMP_ERR_NOERROR) {
			for(vars = response->variables; vars; vars = vars->next_variable) {
				counter_t i = 0;
				if (vars->type == ASN_INTEGER || vars->type == ASN_GAUGE || vars->type == ASN_UINTEGER) {
					i = *vars->val.integer;
				} else if (vars->type == ASN_COUNTER64) {
					i = (((counter_t)(*vars->val.counter64).high) << 32) + (*vars->val.counter64).low;
				}
				if (i != 0) {
					ResultRow r(row.id, i, 0, row.bits);
					rs[row.table].rows.push_back(r);
				}
			}
		} else {
			if (status == STAT_SUCCESS)
				fprintf(stderr, "Error in packet\nReason: %s\n",
				snmp_errstring(response->errstat));
			else
				snmp_sess_perror("snmpget", ss);
		}


		if (response)
			snmp_free_pdu(response);
	}

	snmp_sess_close(sessp);

	return rs;
}

void process_host(QueryHost &host, ResultCache &cache) {
#ifdef MYSQL
	mysqlpp::Connection conn(false);
	if (!conn.connect(DB, SERVER, USER, PASS)) {}
#endif
	time_t now_time = time(NULL);
	std::map<std::string, ResultSet> results = query(host);
	std::map<std::string, ResultSet>::iterator it;
	for (it = results.begin(); it != results.end(); it++) {
		std::pair<std::string, ResultSet> p = *it;
		ResultSet r = p.second;
		if (r.rows.size() > 0) {
			// std::cout << "LOCK TABLE " << r.table << std::endl;
			std::list<ResultRow>::iterator ri;
			std::stringstream query;
			query << "INSERT INTO " << r.table << " (id, dtime, counter, rate) VALUES ";
			int i = 0;
			for (ri = r.rows.begin(); ri != r.rows.end(); ri++) {
				ResultRow row = *ri;
				std::pair<std::string, int> key = std::pair<std::string, int>(r.table, row.id);
				time_t prev_time = cache.times[key];
				if (prev_time != 0) {
					time_t time_diff = now_time - prev_time;
					counter_t prev_counter = cache.counters[key];
					counter_t counter_diff = row.counter - prev_counter;
					counter_t rate = 0;
					if (row.bits == 0)
						rate = row.counter;
					else
						rate = counter_diff / time_diff;
					if (i > 0)
						query << ", ";
					query << "(" << row.id << ", now(), " << counter_diff << ", " << rate << ")";
					i++;
				}
				cache.counters[key] = row.counter;
				cache.times[key] = now_time;
			}
#ifdef MYSQL
			mysqlpp::Query q = conn.query(query.str());
#else
			query.flush();
			std::cout << query.str() << std::endl;
#endif
		}
	}
}

std::vector<QueryHost> load_hosts() {
	std::vector<QueryHost> hosts;
	std::ifstream targets;
	targets.open("targets.easy");
	std::string token;
	QueryHost host;
	QueryRow row;
	int nhosts = 0;
	int ntargs = 0;
	while (targets.good()) {
		targets >> token;
		if (token == "host") {
			// host 172.16.18.9 dkvavalad07 2
			if (host.host != "none")
				hosts.push_back(host);

			std::string ip;
			std::string community;
			int snmpver;
			targets >> ip >> community >> snmpver;
			host = QueryHost(ip, community, snmpver);
			nhosts++;
		}
		else if (token == "target") {
			// target 1.3.6.1.2.1.10.94.1.1.2.1.5.1001005 0 adslAtucCurrSnrAtn_560 7029 30000000
			std::string oid;
			unsigned bits;
			std::string table;
			unsigned id;
			counter_t speed;
			targets >> oid >> bits >> table >> id >> speed;
			row = QueryRow(oid, table, id, bits);
			host.rows.push_back(row);
			ntargs++;
		}
	}
	if (host.host != "none")
		hosts.push_back(host);
	// std::cout << "good: " << targets.good() << " eof: " << targets.eof() << " fail: " << targets.fail() << " bad: " << targets.bad() << std::endl;
	std::cerr << "Loaded " << ntargs << " targets in " << nhosts << " hosts." << std::endl;
	targets.close();
	return hosts;
}

void* thread_loop(void *ptr) {
	unsigned offset = *((unsigned*) ptr);
	unsigned stride = THREADS;
	std::cerr << "Starting thread with offset " << offset << " and stride " << stride << "." << std::endl;
	std::cerr << "We have " << hosts.size() << " hosts to handle." << std::endl;
	while (1) {
		time_t start = time(NULL);
		for (unsigned i = offset; i < hosts.size(); i += stride) {
			QueryHost host = hosts[i];
			process_host(host, cache[i]);
		}
		time_t end = time(NULL);
		time_t sleep_time = INTERVAL - (end - start);
		std::cerr << "Thread " << offset << " sleeping " << sleep_time << " seconds." << std::endl;
		sleep(sleep_time);
	}
	return NULL;
}

int main (int argc, char * const argv[]) {
	init_snmp("betterpoller");

	std::cerr << "Counter_t is " << sizeof(counter_t)*8 << " bits." << std::endl;

	hosts = load_hosts();
	cache = std::vector<ResultCache>(hosts.size());
	pthread_t threads[THREADS];

	for (unsigned i = 0; i < THREADS; i++) {
		pthread_create(&threads[i], NULL, thread_loop, (void*) &i);
		usleep(10);
	}

	for (unsigned i = 0; i < THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	return 0;
}
