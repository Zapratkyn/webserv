#ifndef __WEBSERV_UTILS_HPP__
# define __WEBSERV_UTILS_HPP__

#include <string>
#include <map>
#include <iostream>
#include <sstream>
#include <fstream>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "../Server.hpp"

namespace webserv_utils {

    std::string getServerName(const std::string &, int &, std::map<std::string, Server*>&);
    std::string trim(const std::string &);
    std::string getServerBlock(std::ifstream &);
    void        initSockaddr(struct sockaddr_in &);
    std::string ft_pop_back(std::string);
    void        initTimeval(struct timeval &);
    void        ft_error(int, struct sockaddr_in);

};

#endif