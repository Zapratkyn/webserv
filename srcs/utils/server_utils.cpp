#include "../../include/utils/server_utils.hpp"

using namespace server_utils;

std::string &getOptionName(const std::string &str) { return str.substr(0, str.find_first_of(" \t")); }

std::string &getOptionValue(const std::string &str)
{
    std::string result = "";

    result = &str[str.find_first_of(" \t")];
    result = &result[result.find_first_not_of(" \t")];

    while (result.back() == '{' || result.back() == ';' || result.back() == ' ' || result.back() == '\t')
        result.pop_back();

    return result;
}