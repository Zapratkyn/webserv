#ifndef __HTTP_TCPSERVER_LINUX_HPP__
# define __HTTP_TCPSERVER_LINUX_HPP__

#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> // Since the (int)socket is a file descriptor, we need unistd.h to close it when we are done

class TcpServer {

private:

	int					_socket;
	int 				_port;
	std::string 		_ip_address;
	struct sockaddr_in 	_socketAddr;

	void		init_addr();
	void		listen_log() const;

public:

	TcpServer(std::string ip_address, int port);
	~TcpServer();

	class openSocketException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't open socket"; } };
	class bindException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't bind sockets"; } };
	class listenException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't start listening"; } };

};

#endif