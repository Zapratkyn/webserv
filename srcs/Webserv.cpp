#include "../include/Webserv.hpp"

using namespace webserv_utils;

Webserv::Webserv(const std::string &conf_file) : _socketAddrLen(sizeof(_socketAddr)), _conf(conf_file) { return; }

Webserv::~Webserv() 
{
	// Since servers in the server list are dynamically allocated pointers, we delete each server one by one in the destructor at the end of the program
	for (std::map<std::string, Server*>::iterator it = _server_list.begin(); it != _server_list.end(); it++)
		delete it->second;
	return;
}

/*
- Isolate every server block in the conf file using the brackets
- Find the server's name in the block we just isolated and use it to add an entry in the server list
(If the server has no name or his name is 'webserv_42(_)', we append a number to differienciate them.)
- Send the server block to a parsing function, in the server class so we can use its attributes without getters
*/
void Webserv::parseConf()
{
	/*
	We already know the file exists and is valid from the valid_file function in main.cpp
	So we can open it at construction without checking for fail()
	*/
	std::ifstream 			infile(_conf.c_str());
	std::string				buffer, server_block, server_name;
	int						default_name = 1;
	Server					*server;
	std::vector<int>		port_list;

	while(!infile.eof())
	{
		getline(infile, buffer);
		if (buffer == "server {")
		{
			server_block = getServerBlock(infile);
			server = new Server;
			server_name = getServerName(server_block, default_name, _server_list);
			if (!server->parseServer(server_block, server_name, port_list))
			{
				/*
				If an error occurs, the server will not be added to the webserv's list of servers
				Therefore, we need to delete it here to avoid leaks
				*/
				delete server; 
				throw confFailureException();
			}
			_server_list[server_name] = server;
		}
	}
	infile.close();
}

void Webserv::displayServers()
{
	std::string 						value;
	int									iValue;
	std::vector<int>					port_list;
	std::map<std::string, t_location>	location_list;
	std::vector<std::string>			method_list;

	std::cout << std::endl;

	for (std::map<std::string, Server*>::iterator it = _server_list.begin(); it != _server_list.end(); it++)
	{
		std::cout << "### " << it->first << " ###\n" << std::endl;
		value = it->second->getHost();
		if (value != "")
			std::cout << "Host : " << value << std::endl;
		value = it->second->getIndex();
		if (value != "")
			std::cout << "Index : " << value << std::endl;
		value = it->second->getRoot();
		if (value != "")
			std::cout << "Root : " << value << std::endl;
		iValue = it->second->getBodySize();
		if (iValue >= 0)
			std::cout << "Client max body size : " << iValue << std::endl;
		port_list = it->second->getPorts();
		if (!port_list.empty())
		{
			std::cout << "Ports :\n";
			for (std::vector<int>::iterator it = port_list.begin(); it != port_list.end(); it++)
				std::cout << "  - " << *it << std::endl;
		}
		location_list = it->second->getLocationlist();
		if (!location_list.empty())
		{
			std::cout << "Locations :\n";
			for (std::map<std::string, t_location>::iterator it = location_list.begin(); it != location_list.end(); it++)
			{
				std::cout << "  - " << it->second.location << " :\n";
				value = it->second.root;
				if (value != "")
					std::cout << "    - Root : " << value << std::endl;
				value = it->second.index;
				if (value != "")
					std::cout << "    - Index : " << value << std::endl;
				method_list = it->second.methods;
				if (!method_list.empty())
				{
					std::cout << "    - Allowed methods :\n";
					for (std::vector<std::string>::iterator it = method_list.begin(); it != method_list.end(); it++)
						std::cout << "       - " << *it << std::endl;
				}
			}
		}
		std::cout << std::endl;
	}
}

void Webserv::startServer()
{
	std::vector<int> port_list;

	initSockaddr(_socketAddr);
	initTimeval(_timeval);
	FD_ZERO(&_readfds);
	FD_ZERO(&_writefds);

	for (std::map<std::string, Server*>::iterator server_it = _server_list.begin(); server_it != _server_list.end(); server_it++)
	{
		port_list = server_it->second->getPorts();
		for (std::vector<int>::iterator port_it = port_list.begin(); port_it != port_list.end(); port_it++)
		{
			_socket = socket(AF_INET, SOCK_STREAM, 0);
			if (_socket < 0)
				throw openSocketException();
			if (FD_ISSET(_socket, &_readfds))
				throw duplicateSocketException();
			
			FD_SET(_socket, &_readfds);
			FD_SET(_socket, &_writefds);
			_socket_list.push_back(_socket);
			server_it->second->addSocket(_socket);
			
			_socketAddr.sin_port = *port_it;
			if (bind(_socket, (sockaddr *)&_socketAddr, _socketAddrLen) < 0)
				throw bindException();
		}
	}
}

void Webserv::startListen()
{
	for (std::vector<int>::iterator it = _socket_list.begin(); it != _socket_list.end(); it++)
	{
		if (listen(*it, 10) < 0)
			throw listenException();
	}

	int max_fds = _socket_list.size() + 3;

	while (true)
	{
		std::cout << "Waiting for new connection...\n\n" << std::endl;
		select(max_fds, &_readfds, &_writefds, NULL, &_timeval);
		if (!newConnection())
			std::cerr << "Server failed to accept incoming connection from ADDRESS: " << 
			inet_ntoa(_socketAddr.sin_addr) << "; PORT: " << 
			ntohs(_socketAddr.sin_port) << std::endl;
	}
}

// void Webserv::startListen()
// {
// 	if (listen(_socket, 10) < 0) // Start listening on the socket, with a maximum of 10 connections at a time
// 		throw listenException();
	
// 	listenLog(); // Display informations about the listening socket

// 	int bytesReceived;

// 	while (true)
// 	{
// 		std::cout << "Waiting for new connection...\n\n" << std::endl;
// 		if (!newConnection(_new_socket))
// 			std::cerr << "Server failed to accept incoming connection from ADDRESS: " << 
// 			inet_ntoa(_socketAddr.sin_addr) << "; PORT: " << 
// 			ntohs(_socketAddr.sin_port) << std::endl;
// 		else
// 		{
// 			char buffer[30720] = {0};
//            	bytesReceived = read(_new_socket, buffer, 30720); // since we are using AF_INET, the requests will be streams we can read()
//             if (bytesReceived < 0)
// 				std::cerr << "Failed to read bytes from client socket connection" << std::endl;
// 			else
// 			{
// 				std::cout << "Request received from the client\n" << std::endl;
// 				sendResponse();
// 			}
// 			close(_new_socket);
// 		}
// 	}
// }

bool Webserv::newConnection()
{
	_new_socket = accept(_socket, (sockaddr *)&_socketAddr, &_socketAddrLen); // _socket is the listening socket, never changes. _socketAddr is teh struct used for each new connection
	if (_new_socket < 0)
		return false;
	return true;
}

// void Webserv::sendResponse()
// {
//     unsigned long bytesSent;

//     bytesSent = write(_new_socket, _serverMessage.c_str(), _serverMessage.size());

//     if (bytesSent == _serverMessage.size())
//         std::cout << "------ Server Response sent to client ------\n" << std::endl;
//     else
//         std::cerr << "Error sending response to client" << std::endl;
// }

// std::string Webserv::buildResponse()
// {
// 	std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>";
//     std::ostringstream ss;
//     ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
//        << htmlFile;

// 	return ss.str();
// }

// void Webserv::initAddr()
// {
// 	_socketAddr.sin_family = AF_INET; // The socket's address family
// 	_socketAddr.sin_port = htons(_port); // Copies the port number. htons ensures the bytes order is respected (stands for Host to Network Short)
// 	_socketAddr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY = "0.0.0.0"
// }
// void Webserv::listenLog() const
// {
// 	std::ostringstream ss;
//    	ss << "\n*** Listening on ADDRESS: " 
//     << inet_ntoa(_socketAddr.sin_addr)  // inet_ntoa converts the Internet Host address to an IPv4 address (xxx.xxx.xxx.xxx)
//     << " PORT: " << ntohs(_socketAddr.sin_port)  // Copies the port number. ntohs ensures the bytes order is respected (stands for Network to Host Short)
//     << " ***\n\n";
// 	std::cout << ss.str() << std::endl;
// }
