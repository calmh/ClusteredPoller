#include <string>
#include <list>
#include <sys/stat.h>
#include <cstdio>

#include "types.h"
#include "util.h"

using namespace std;

// Detach from console. Common recipe.
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

	FILE *ignored;
        ignored = freopen( "/dev/null", "r", stdin);
        ignored = freopen( "/dev/null", "w", stdout);
        ignored = freopen( "/dev/null", "w", stderr);
}

// Remove one semicolon from a string.
string no_semi(string token)
{
        size_t semicolon = token.find(';', 0);
        if (semicolon != string::npos)
                token = token.erase(semicolon, 1);
        return token;
}

// Convert characters in a range to lower case.
template <typename Iter> void range_tolower (Iter beg, Iter end)
{
       for ( Iter iter = beg; iter != end; ++iter ) {
                *iter = std::tolower( *iter );
        }
}

// Convert an entire string to lower case, in place.
void string_tolower (string& str)
{
        range_tolower(str.begin(), str.end());
}

string string_uncomment(string& line)
{
        string replaced(line);

        // Remove comment
        string::size_type position = replaced.find("#", 0);
        if (position != string::npos)
                replaced.erase(position, replaced.length() - position);

        // Remove space at the end
        position = replaced.find_last_not_of(" ", replaced.length() - 1);
        if (position != replaced.length() - 1 && position != string::npos)
                replaced.erase(position + 1, replaced.length() - position);

        // Remove space at the beginning
        position = replaced.find_first_not_of(" ", 0);
        if (position != 0 && position != string::npos)
                replaced.erase(0, position);

        return replaced;
}

list<string> string_split(string& line, const char* separator)
{
        list<string> parts;
        string::size_type start = 0;
        string::size_type end = line.find(separator, start);
        while (end != string::npos) {
                string part = line.substr(start, end - start);
                parts.push_back(part);
                start = end + 1;
                end = line.find(separator, start);
        }
        string part = line.substr(start, line.length() - start);
        parts.push_back(part);
        return parts;
}
