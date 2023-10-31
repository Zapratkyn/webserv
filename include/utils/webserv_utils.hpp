#ifndef __WEBSERV_UTILS_HPP__
# define __WEBSERV_UTILS_HPP__

#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/ip.h>
#include <ctime>
#include "../Server.hpp"
#include "utils.hpp"

# define DISPLAY_REQUEST false
# define DISPLAY_METHOD_AND_LOCATION false


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
    void        getRequest(int, struct t_request &);
    void		setRequest(t_request &, std::vector<std::string> &);
    bool		validMethod(std::string &);
    void		killMessage(int);
    void        errorPage(struct t_request);
    void        initRequest(struct t_request&);

    class readRequestException : public std::exception { public: virtual const char *what() const throw() { return "Error while reading request"; } };
    class requestBodyTooBigException : public std::exception { public: virtual const char *what() const throw() { return "Client's request's body too big"; } };
    class invalidMethodException : public std::exception { public: virtual const char *what() const throw() { return "Invalid method"; } };
	class forbiddenMethodException : public std::exception { public: virtual const char *what() const throw() { return "Forbidden method"; } };

};

#endif
