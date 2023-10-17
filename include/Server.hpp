#ifndef __SERVER_HPP__
# define __SERVER_HPP__

#include <iostream>
#include <netinet/in.h>

class Server {

private:

	int								_socket;
	int								_new_socket;
	int 							_port;
	struct sockaddr_in 				_socketAddr;
	unsigned int					_socketAddrLen;
	std::string						_serverMessage;

public:

	Server();
	~Server();

};

#endif