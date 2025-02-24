#ifndef __UTILS__
#define __UTILS__

#include <vector>
#include <string>


namespace utils
{
    static const std::vector<std::vector<std::string>> operatorSplit = { {"<=", "<", ">=", ">", "==", "!="}, // num comparisons
                  {"||", "&&", "==", "!="} }; // bool comparisons

    enum operationType{
        FLOAT,
        INTEGER,
        BOOL
    };
} // namespace utils



#endif