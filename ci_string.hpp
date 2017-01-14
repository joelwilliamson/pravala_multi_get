#ifndef CI_STRING_HPP
#define CI_STRING_HPP

#include <string>

namespace ci
{
        // Case-insensitive characters, copied from
        // http://en.cppreference.com/w/cpp/string/char_traits
        struct ci_char_traits : public std::char_traits<char> {
                static bool eq(char c1, char c2) {
                        return std::toupper(c1) == std::toupper(c2);
                }
                static bool lt(char c1, char c2) {
                        return std::toupper(c1) <  std::toupper(c2);
                }
                static int compare(const char* s1, const char* s2, size_t n) {
                        while ( n-- != 0 ) {
                                if ( std::toupper(*s1) < std::toupper(*s2) ) return -1;
                                if ( std::toupper(*s1) > std::toupper(*s2) ) return 1;
                                ++s1; ++s2;
                        }
                        return 0;
                }
                static const char* find(const char* s, int n, char a) {
                        auto const ua (std::toupper(a));
                        while ( n-- != 0 ) 
                        {
                                if (std::toupper(*s) == ua)
                                        return s;
                                s++;
                        }
                        return nullptr;
                }
        };
        using string = std::basic_string<char, ci_char_traits>;

        string from_string(const std::string& s);
        std::string to_string(const string& s);
        std::ostream& operator<<(std::ostream& os, string s);
}

#endif
