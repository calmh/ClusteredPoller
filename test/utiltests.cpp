#include <string>
#include "UnitTest++.h"
#include "util.h"
using namespace std;

SUITE(QuickTests)
{
        TEST(UncommentOneLine)
        {
                string line("# just a comment");
                CHECK_EQUAL("", string_uncomment(line));
                line = "text before# comment";
                CHECK_EQUAL("text before", string_uncomment(line));
                line = "   text before   # comment";
                CHECK_EQUAL("text before", string_uncomment(line));
        }

        TEST(SplitOneLineIntoParts)
        {
                string line("two\twords ");
                list<string> parts = string_split(line, "\t");
                CHECK_EQUAL((size_t)2, parts.size());
                CHECK_EQUAL("two", parts.front()); parts.pop_front();
                CHECK_EQUAL("words ", parts.front());
        }

        TEST(SplitOneComplexLineIntoParts)
        {
                string line = "more words\t separated\tby tabs \t";
                list<string> parts = string_split(line, "\t");
                CHECK_EQUAL((size_t)4, parts.size());
                CHECK_EQUAL("more words", parts.front()); parts.pop_front();
                CHECK_EQUAL(" separated", parts.front()); parts.pop_front();
                CHECK_EQUAL("by tabs ", parts.front()); parts.pop_front();
                CHECK_EQUAL("", parts.front());
        }
}
