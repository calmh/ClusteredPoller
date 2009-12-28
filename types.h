#ifndef _TYPES_H
#define _TYPES_H

#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <map>
#include <vector>

#ifdef USE_MYSQL
#include <mysql++.h>
#endif

struct RTGConf {
	unsigned interval;
	unsigned threads;
	double high_skew_slop;
	double low_skew_slop;
	std::string dbhost;
	std::string database;
	std::string dbuser;
	std::string dbpass;

	RTGConf() {
		interval = 300;
		threads = 2;
		high_skew_slop = 3.0;
		low_skew_slop = 0.5;
	}
};

struct ResultCache {
	std::map<std::pair<std::string, int>, uint64_t> counters;
	std::map<std::pair<std::string, int>, time_t> times;
};

struct ResultRow {
	int id;
	uint64_t counter;
	uint64_t rate;
	int bits;
	time_t dtime;
	uint64_t speed;

	ResultRow(int iid, uint64_t icounter, uint64_t irate, int ibits, time_t idtime, uint64_t ispeed) {
		id = iid;
		counter = icounter;
		rate = irate;
		bits = ibits;
		dtime = idtime;
		speed = ispeed;
	}
};

struct ResultSet {
	std::string table;
	std::vector<ResultRow> rows;

	ResultSet() {}
	ResultSet(std::string itable) {
		table = itable;
	}
};

struct QueryRow {
	std::string oid;
	std::string table;
	unsigned id;
	unsigned bits;
	uint64_t speed;

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
	std::vector<QueryRow> rows;

	QueryHost() {
		host = "none";
	}
	QueryHost(std::string ihost, std::string icommunity, int isnmpver) {
		host = ihost;
		community = icommunity;
		snmpver = isnmpver;
	}
};

#endif
