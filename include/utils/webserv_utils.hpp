#ifndef __WEBSERV_UTILS_HPP__
# define __WEBSERV_UTILS_HPP__

#include <string>
#include <fstream>

namespace webserv_utils {

    std::string &getServerName(const std::string &);
    std::string &trim(const std::string &);

};

#endif