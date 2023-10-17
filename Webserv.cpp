#include "Webserv.hpp"

Webserv::Webserv(int port) : _port(port), _socketAddrLen(sizeof(_socketAddr)), _serverMessage(buildResponse())
{
	return;
}
Webserv::~Webserv() 
{
	if (_socket >= 0)
		close(_socket);
	if (_new_socket >= 0)
		close(_new_socket);
	return;
}

void Webserv::startServer()
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
}

void Webserv::startListen()
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
				std::cout << "Request received from the client\n" << std::endl;
				sendResponse();
			}
			close(_new_socket);
		}
	}
}

bool Webserv::newConnection(int &new_socket)
{
	new_socket = accept(_socket, (sockaddr *)&_socketAddr, &_socketAddrLen); // _socket is the listening socket, never changes. _socketAddr is teh struct used for each new connection
	if (new_socket < 0)
		return false;
	return true;
}

void Webserv::sendResponse()
{
    unsigned long bytesSent;

    bytesSent = write(_new_socket, _serverMessage.c_str(), _serverMessage.size());

    if (bytesSent == _serverMessage.size())
        std::cout << "------ Server Response sent to client ------\n" << std::endl;
    else
        std::cerr << "Error sending response to client" << std::endl;
}

std::string Webserv::buildResponse()
{
	std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>";
    std::ostringstream ss;
    ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
       << htmlFile;

	return ss.str();
}

void Webserv::initAddr()
{
	_socketAddr.sin_family = AF_INET; // The socket's address family
	_socketAddr.sin_port = htons(_port); // Copies the port number. htons ensures the bytes order is respected (stands for Host to Network Short)
	_socketAddr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY = "0.0.0.0"
}
void Webserv::listenLog() const
{
	std::ostringstream ss;
   	ss << "\n*** Listening on ADDRESS: " 
    << inet_ntoa(_socketAddr.sin_addr)  // inet_ntoa converts the Internet Host address to an IPv4 address (xxx.xxx.xxx.xxx)
    << " PORT: " << ntohs(_socketAddr.sin_port)  // Copies the port number. ntohs ensures the bytes order is respected (stands for Network to Host Short)
    << " ***\n\n";
	std::cout << ss.str() << std::endl;
}
