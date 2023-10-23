#ifndef __WEBSERV_HPP__
# define __WEBSERV_HPP__

// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <unistd.h>
#include "utils/webserv_utils.hpp"

class Webserv {

private:

	// std::string 					_ip_address;
	int								_socket;
	// fd_set							_socket_list;
	// int							_new_socket;
	// int 							_port;
	// struct sockaddr_in 				_socketAddr;
	unsigned int					_socketAddrLen;
	// std::string					_serverMessage;

	// void							initAddr();
	// void							listenLog() const;
	// bool							newConnection(int&);
	// std::string					buildResponse();
	// void							sendResponse();

	std::string						_conf;
	std::map<std::string, Server*>	_server_list;

public:

	Webserv(const std::string &);
	~Webserv();
	// void	startListen();
	void	startServer();
	void	parseConf();
	void	displayServers();

	class openSocketException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't open socket"; } };
	class duplicateSocketException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nSocket already exists"; } };
	class bindException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't bind sockets"; } };
	class listenException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't start listening"; } };
	class confFailureException : public std::exception { public: virtual const char *what() const throw() { return "Configuration failure. Program stopped."; } };

};

#endif
