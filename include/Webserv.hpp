#ifndef __WEBSERV_HPP__
# define __WEBSERV_HPP__

#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // Since the (int)socket is a file descriptor, we need unistd.h to close it when we are done
#include <map>
#include <fstream>
#include <string>
#include "Server.hpp"

class Webserv {

private:

	std::string 					_ip_address;
	int								_socket;
	int								_new_socket;
	int 							_port;
	struct sockaddr_in 				_socketAddr;
	unsigned int					_socketAddrLen;
	std::string						_serverMessage;
	std::string						_conf;
	std::map<std::string, Server*>	_server_list;

	void		initAddr();
	void		listenLog() const;
	bool		newConnection(int&);
	std::string	buildResponse();
	void		sendResponse();

public:

	Webserv(std::string);
	~Webserv();
	void	startListen();
	void	startServer();
	void	parseConf();
	std::string getServerName(std::string) const;
	Server *parseServer(std::string) const;

	class openSocketException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't open socket"; } };
	class bindException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't bind sockets"; } };
	class listenException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't start listening"; } };

};

#endif
