typedef unsigned long long counter_t;

struct ResultCache {
	std::map<std::pair<std::string, int>, counter_t> counters;
	std::map<std::pair<std::string, int>, time_t> times;
};

struct ResultRow {
	int id;
	counter_t counter;
	counter_t rate;
	int bits;
	time_t dtime;

	ResultRow(int iid, counter_t icounter, counter_t irate, int ibits, time_t idtime) {
		id = iid;
		counter = icounter;
		rate = irate;
		bits = ibits;
		dtime = idtime;
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

