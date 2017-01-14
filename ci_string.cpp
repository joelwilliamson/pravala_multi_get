#include "ci_string.hpp"

namespace ci
{
        ci::string from_string(const std::string& s)
        {
                return ci::string(s.cbegin(), s.cend());
        }
        
        std::string to_string(const ci::string& s)
        {
                return std::string(s.cbegin(), s.cend());
        }
        
        std::ostream& operator<<(std::ostream& os, ci::string s)
        {
                return os << to_string(s);
        }
}
