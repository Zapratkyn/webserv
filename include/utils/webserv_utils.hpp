#ifndef __WEBSERV_UTILS_HPP__
# define __WEBSERV_UTILS_HPP__

#include "../Server.hpp"
#include "utils.hpp"


namespace webserv_utils {

    bool        			checkRedirectionList(std::vector<std::string> &);
    std::string 			getServerBlock(std::ifstream &);
    void        			ft_error(int, std::string);
	void					getPotentialServers(std::vector<Server*>&, struct sockaddr_in&, struct t_request &);
	void					getServer(struct t_request &);
	void					displayServers(std::vector<Server*>&);
    void        			parseUrl(std::string, std::vector<std::string> &, std::vector<std::string> &);
	void					readRequests(std::map<std::string, Server*> &, std::map<int, t_request> &, fd_set &);
    void        			getRequest(struct t_request &);
    bool					validMethod(std::string &);
    void        			initRequest(struct t_request&);
	bool					socketIsSet(std::map<int, struct sockaddr_in> &, struct sockaddr_in &);

    class readRequestException : public std::exception { public: virtual const char *what() const throw() { return "Error while reading request"; } };
    class requestBodyTooBigException : public std::exception { public: virtual const char *what() const throw() { return "Client's request's body too big"; } };
	class forbiddenMethodException : public std::exception { public: virtual const char *what() const throw() { return "Forbidden method"; } };

};

#endif
