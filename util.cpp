#include "util.h"

using namespace std;

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
