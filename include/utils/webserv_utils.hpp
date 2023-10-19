#ifndef __WEBSERV_UTILS_HPP__
# define __WEBSERV_UTILS_HPP__

#include <string>

namespace webserv_utils {

    std::string trim(std::string);
    std::string getOptionName(std::string);
    std::string getOptionValue(std::string);

};

#endif