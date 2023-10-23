#ifndef __WEBSERV_HPP__
# define __WEBSERV_HPP__

#include <unistd.h>
#include "utils/webserv_utils.hpp"

# define EXIT_SUCCESS 0
# define EXIT_FAILURE 1

class Webserv {

private:

	int								_socket;
	fd_set							_readfds;
	fd_set							_writefds;
	std::vector<int>				_socket_list;
	int								_new_socket;
	struct sockaddr_in 				_socketAddr;
	unsigned int					_socketAddrLen;
	struct timeval					_timeval;

	bool							newConnection();

	std::string						_conf;
	std::map<std::string, Server*>	_server_list;

public:

	Webserv(const std::string &);
	~Webserv();
	void	startListen();
	void	startServer();
	void	parseConf();
	void	displayServers();
	void	listenLog();

	class openSocketException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't open socket"; } };
	class duplicateSocketException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nSocket already exists"; } };
	class bindException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't bind socket"; } };
	class listenException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't start listening"; } };
	class confFailureException : public std::exception { public: virtual const char *what() const throw() { return "Configuration failure. Program stopped."; } };

};

#endif
