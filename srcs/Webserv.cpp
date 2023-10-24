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
			/*
			Since we cannot iterate on a fd_set, 
			we keep track of all the open sockets so we can close them all later
			*/
			_socket_list.push_back(_socket);
			server_it->second->addSocket(_socket);
			
			_socketAddr.sin_port = htons(*port_it);
			if (bind(_socket, (sockaddr *)&_socketAddr, _socketAddrLen) < 0)
				throw bindException();
		}
	}
}

void Webserv::startListen()
{
	for (std::vector<int>::iterator it = _socket_list.begin(); it != _socket_list.end(); it++)
	{
		if (listen(*it, 1000) < 0)
			throw listenException();
	}

	listenLog();

	/*
	Select() needs the biggest fd + 1 from all the fd_sets
	In our case, readfds and writefds contain the same fd's
	Since fd 1 and 2 are already taken (STD_IN and STD_OUT), our list begins at 3
	Therefore, max_fds = total_number_of_sockets + STD_IN + STD_OUT + 1
	*/
	int max_fds = _socket_list.size() + 3;

	while (true)
	{
		std::cout << "Waiting for new connection...\n\n" << std::endl;
		_socket = select(max_fds, &_readfds, &_writefds, NULL, 0); // Incorrect !!!
		if (!_socket)
		{
			ft_error(0, _socketAddr);
			continue;
		}
		if (!newConnection())
		{
			ft_error(1, _socketAddr);
			continue;
		}
		close(_new_socket);
		/*
		We need to find a way to stop the program properly
		If we press CTRL-C, we kill it and leaks happen
		Maybe have a web page with a dedicated button...
		(Like close_server.html, with a button sending a specific message in the client_body)
		*/
	}
	for (std::vector<int>::iterator it = _socket_list.begin(); it != _socket_list.end(); it++)
		close(*it);
}

bool Webserv::newConnection()
{
	_new_socket = accept(_socket, (sockaddr *)&_socketAddr, &_socketAddrLen);
	if (_new_socket < 0)
		return false;
	return true;
}

void Webserv::listenLog()
{
	std::ostringstream 	ss;
	std::vector<int>	port_list;
   	ss << "### Webserv started ###\n\n"
	<< "\n***\n\nListening on ADDRESS: " 
    << inet_ntoa(_socketAddr.sin_addr)  // inet_ntoa converts the Internet Host address to an IPv4 address (xxx.xxx.xxx.xxx)
    << "\n\nPORTS:\n\n";
	for (std::map<std::string, Server*>::iterator server_it = _server_list.begin(); server_it != _server_list.end(); server_it++)
	{
		port_list = server_it->second->getPorts();
		for (std::vector<int>::iterator port_it = port_list.begin(); port_it != port_list.end(); port_it++)
			ss << " - " << *port_it << "\n";
	}
    ss << "\n***\n\n";
	std::cout << ss.str() << std::endl;
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
