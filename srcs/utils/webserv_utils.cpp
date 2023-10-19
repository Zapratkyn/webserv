#include "../../include/utils/webserv_utils.hpp"

using namespace webserv_utils;

std::string trim(std::string str)
{
    std::string result = "";
    int pos;

    if (str.size())
    {
        pos = str.find_first_not_of(" \t");
        result = &str[pos];
        pos = result.size() - 1;
        while (result[pos] == ' ' || result[pos] == '\t')
        {
            result.pop_back();
            pos--;
        }
    }
    return result;
}
std::string getOptionName(std::string str) { return str.substr(0, str.find_first_of(" \t")); }
std::string getOptionValue(std::string str)
{
    std::string result = "";

    result = &str[str.find_first_of(" \t")];
    result = &result[result.find_first_not_of(" \t")];

    while (result.back() == '{' || result.back() == ';' || result.back() == ' ' || result.back() == '\t')
        result.pop_back();

    return result;
}