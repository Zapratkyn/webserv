#ifndef __WEBSERV_UTILS_HPP__
# define __WEBSERV_UTILS_HPP__

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/ip.h>
#include "../Server.hpp"

namespace webserv_utils {

    std::string getServerName(const std::string &, int &, std::map<std::string, Server*>&);
    std::string getServerBlock(std::ifstream &);
    void        initSockaddr(struct sockaddr_in &);
    // void        initTimeval(struct timeval &);
    void        ft_error(int);
	void		listenLog(struct sockaddr_in&, std::map<std::string, Server*>&);
	std::string	getServer(std::map<std::string, Server*>&, int&);
	void		displayServers(std::map<std::string, Server*>&);
    void        parseUrl(std::string, std::vector<std::string> &);

    // libft
    std::string ft_pop_back(std::string);
    int         ft_stoi(std::string);
	std::string ft_to_string(int);
    std::string trim(const std::string &);

};

#endif
