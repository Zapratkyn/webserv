#include "http_tcpServer_linux.hpp"

TcpServer::TcpServer(std::string ip_address, int port) : _ip_address(ip_address), _port(port), _socketAddrLen(sizeof(_socketAddr))
{
	/*Creating a socket, a communication entry point. 
	AF_INET for TCP/IP protocol. AF stands for Address Family (in this case, IPv4)
	SOCK_STREAM is the type of communication > Safe binary stream
	0 is the default protocol*/
	_socket = socket(AF_INET, SOCK_STREAM, 0); 
	if (_socket < 0)
		throw openSocketException();
	
	initAddr(); // Since the _socketAddr is a data structure, it needs to be set for incoming uses

	if (bind(_socket, (sockaddr *)&_socketAddr, _socketAddrLen) < 0) // Binding the socket to the address
		throw bindException();

	return;
}
TcpServer::~TcpServer() 
{
	if (_socket >= 0)
		close(_socket);
	if (_new_socket >= 0)
		close(_new_socket);
	return;
}

void TcpServer::startListen()
{
	if (listen(_socket, 10) < 0) // Start listening on the socket, with a maximum of 10 connections at a time
		throw listenException();
	
	listenLog(); // Display informations about the listening socket

	int bytesReceived;

	while (true)
	{
		std::cout << "Waiting for new connection...\n\n" << std::endl;
		if (!newConnection(_new_socket))
			std::cerr << "Server failed to accept incoming connection from ADDRESS: " << 
			inet_ntoa(_socketAddr.sin_addr) << "; PORT: " << 
			ntohs(_socketAddr.sin_port) << std::endl;
		else
		{
			char buffer[30720] = {0};
           	bytesReceived = read(_new_socket, buffer, 30720); // since we are using AF_INET, the requests will be streams we can read()
            if (bytesReceived < 0)
				std::cerr << "Failed to read bytes from client socket connection" << std::endl;
			else
			{
				std::cout << "Request received from the client" << std::endl;
				// sendResponse();
			}
			close(_new_socket);
		}
	}
}

bool TcpServer::newConnection(int &new_socket)
{
	new_socket = accept(_socket, (sockaddr *)&_socketAddr, &_socketAddrLen); // _socket and _socketAddr are free to be used again since our server is listening on the _socket already
	if (new_socket < 0)
		return false;
	return true;
}

void TcpServer::initAddr()
{
	_socketAddr.sin_family = AF_INET; // The socket's address family
	_socketAddr.sin_port = htons(_port); // Copies the port number. htons ensures the bytes order is respected (stands for Host to Network Short)
	_socketAddr.sin_addr.s_addr = inet_addr(_ip_address.c_str()); // inet_addr converts host's IP into binary
}
void TcpServer::listenLog() const
{
	std::ostringstream ss;
   	ss << "\n*** Listening on ADDRESS: " 
    << inet_ntoa(_socketAddr.sin_addr)  // inet_ntoa converts the Internet Host address to an IPv4 address (xxx.xxx.xxx.xxx)
    << " PORT: " << ntohs(_socketAddr.sin_port)  // Copies the port number. ntohs ensures the bytes order is respected (stands for Network to Host Short)
    << " ***\n\n";
	std::cout << ss.str() << std::endl;
}
