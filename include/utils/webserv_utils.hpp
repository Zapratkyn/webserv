#ifndef __WEBSERV_UTILS_HPP__
# define __WEBSERV_UTILS_HPP__

#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
// #include <sys/socket.h>
// #include <sys/select.h>
#include "../Server.hpp"

namespace webserv_utils {

    std::string getServerName(const std::string &, int &, std::map<std::string, Server*>&);
    std::string trim(const std::string &);
    std::string getServerBlock(std::ifstream &);
    // void        initSockaddr(struct sockaddr_in &);

};

#endif