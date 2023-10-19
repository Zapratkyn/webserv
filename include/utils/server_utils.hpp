#ifndef __SERVER_UTILS_HPP__
# define __SERVER_UTILS_HPP__

#include <iostream>
#include <string>
#include <fstream>

namespace server_utils {

    std::string &getOptionName(const std::string &);
    std::string &getOptionValue(const std::string &);

};

#endif