#ifndef _QUERY_H
#define _QUERY_H

#include "types.h"

std::map<std::string, ResultSet> query(QueryHost qh);
void process_host(QueryHost &host, ResultCache &cache);

#endif
