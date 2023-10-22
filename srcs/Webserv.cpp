#include "../include/Webserv.hpp"

using namespace webserv_utils;

Webserv::Webserv(const std::string &conf_file) : _conf(conf_file) { return; }

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
(If the server has no name or his name is 'webserv_42(_)', we append a number to differienciate them. 
This way, we can use the same port for several servers)
- Send the server block to a parsing function, in the server class so we can use its attributes without getters
*/
void Webserv::parseConf()
{
	/*
	We already know the file exists and is valid from the valid_file function in main.cpp
	So we can open it at construction without checking for fail()
	*/
	std::ifstream 	infile(_conf);
	std::string		buffer, server_block, server_name;
	size_t			default_name = 1;
	Server			*server;

	while(!infile.eof())
	{
		getline(infile, buffer);
		if (buffer == "server {")
		{
			server_block = getServerBlock(infile);
			server = new Server;
			server_name = getServerName(server_block);
			if (server_name == "webserv_42")
				server_name.append("_");
			if (server_name == "webserv_42_")
				server_name = server_name.append(std::to_string(default_name++));
			if (!server->parseServer(server_block, server_name))
			{
				/*
				If an error occurs, the server will not be added to the webserv's list of servers
				Therefore, we need to delete it here to avoid leaks
				*/
				delete server; 
				throw wrongOptionException();
			}
			_server_list[server_name] = server;
		}
	}
	infile.close();
}

void Webserv::display_servers()
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
			std::cout << "Client max bidy size : " << iValue << std::endl;
		port_list = it->second->getPorts();
		if (!port_list.empty())
		{
			std::cout << "Ports :\n";
			for (std::vector<int>::iterator it = port_list.begin(); it != port_list.end(); it++)
				std::cout << "	- " << *it << std::endl;
		}
		location_list = it->second->getLocationlist();
		if (!location_list.empty())
		{
			std::cout << "Locations :\n";
			for (std::map<std::string, t_location>::iterator it = location_list.begin(); it != location_list.end(); it++)
			{
				std::cout << "	- " << it->second.location << " :\n";
				value = it->second.root;
				if (value != "")
					std::cout << "		- Root : " << value << std::endl;
				value = it->second.index;
				if (value != "")
					std::cout << "		- Index : " << value << std::endl;
				method_list = it->second.methods;
				if (!method_list.empty())
				{
					std::cout << "		- Allowed methods :\n";
					for (std::vector<std::string>::iterator it = method_list.begin(); it != method_list.end(); it++)
						std::cout << "			- " << *it << std::endl;
				}
			}
		}
		std::cout << std::endl;
	}
}

// void Webserv::startServer()
// {
// 	/*Creating a socket, a communication entry point. 
// 	AF_INET for TCP/IP protocol. AF stands for Address Family (in this case, IPv4)
// 	SOCK_STREAM is the type of communication > Safe binary stream
// 	0 is the default protocol*/
// 	_socket = socket(AF_INET, SOCK_STREAM, 0); 
// 	if (_socket < 0)
// 		throw openSocketException();
	
// 	initAddr(); // Since the _socketAddr is a data structure, it needs to be set for incoming uses

// 	if (bind(_socket, (sockaddr *)&_socketAddr, _socketAddrLen) < 0) // Binding the socket to the address
// 		throw bindException();
// }

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

// bool Webserv::newConnection(int &new_socket)
// {
// 	new_socket = accept(_socket, (sockaddr *)&_socketAddr, &_socketAddrLen); // _socket is the listening socket, never changes. _socketAddr is teh struct used for each new connection
// 	if (new_socket < 0)
// 		return false;
// 	return true;
// }

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
