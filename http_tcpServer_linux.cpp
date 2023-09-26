#include "http_tcpServer_linux.hpp"

TcpServer::TcpServer(std::string ip_address, int port) : _ip_address(ip_address), _port(port)
{
	/*Creating a socket, a communication entry point. 
	AF_INET for TCP/IP protocol. AF stands for Address Family (in this case, IPv4)
	SOCK_STREAM is the type of communication > Safe binary stream
	0 is the default protocol*/
	_socket = socket(AF_INET, SOCK_STREAM, 0); 
	if (_socket < 0)
		throw openSocketException();
	
	init_addr(); // Since the _socketAddr is a data structure, it needs to be set for incoming uses

	if (bind(_socket, (sockaddr *)&_socketAddr, sizeof(_socketAddr)) < 0) // Binding the socket to the address
		throw bindException();
	
	if (listen(_socket, 10) < 0) // Start listening on the socket, with a maximum of 10 connections at a time
		throw listenException();
	
	listen_log(); // Display informations about the listening socket
	return;
}
TcpServer::~TcpServer() 
{
	if (_socket >= 0)
		close(_socket);
	return;
}

void TcpServer::init_addr()
{
	_socketAddr.sin_family = AF_INET; // The socket's address family
	_socketAddr.sin_port = htons(_port); // Copies the port number. htons ensures the bytes order is respected (stands for Host to Network Short)
	_socketAddr.sin_addr.s_addr = inet_addr(_ip_address.c_str()); // inet_addr converts host's IP into binary
}
void TcpServer::listen_log() const
{
	std::ostringstream ss;
    ss << "\n*** Listening on ADDRESS: " 
        << inet_ntoa(_socketAddr.sin_addr)  // inet_ntoa converts the Internet Host address to an IPv4 address (xxx.xxx.xxx.xxx)
        << " PORT: " << ntohs(_socketAddr.sin_port)  // // Copies the port number. ntohs ensures the bytes order is respected (stands for Network to Host Short)
        << " ***\n\n";
	std::cout << ss.str() << std::endl;
}