#ifndef __HTTP_TCPSERVER_LINUX_HPP__
# define __HTTP_TCPSERVER_LINUX_HPP__

#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

class TcpServer {

private:

	int			_socket;
	int 		_port;
	std::string _ip_address;

public:

	TcpServer(std::string ip_address, int port);
	~TcpServer();

	int getSocket() const;

};

#endif