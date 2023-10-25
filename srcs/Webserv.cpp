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
	displayServers(_server_list);
}

void Webserv::startServer()
{
	std::vector<int> port_list;

	initSockaddr(_socketAddr);
	// initTimeval(_tv);
	FD_ZERO(&_readfds);


	/*
	Each port in the conf file is used to make an individual listening socket
	We browse the whole list, create a socket for each port
	We add the socket to the readfds set (for select()), to its server's socket_list (for getServer()) \
	and to the global socket_list (to close everything at the end)
	Bind() gives a "name" to each socket
	*/
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

	listenLog(_socketAddr, _server_list); // Displays all the open ports to the user

	/*
	Select() needs the biggest fd + 1 from all the fd_sets
	Since fd 0, 1 and 2 are already taken (STD_IN and STD_OUT and STD_ERR), our list begins at 3
	Therefore, max_fds = total_number_of_sockets + STD_IN + STD_OUT + STD_ERR
	*/
	int max_fds = _socket_list.size() + 3;
	std::string server, message;

	while (true)
	{
		std::cout << "Waiting for new connection...\n" << std::endl;
		if (select(max_fds, &_readfds, NULL, NULL, NULL) < 0) // Blocks until a new request is received
		{
			ft_error(0, _socketAddr);
			continue;
		}
		_new_socket = newConnection(max_fds);
		if (_new_socket < 0)
		{
			ft_error(1, _socketAddr);
			continue;
		}
		server = getServer(_server_list, _socket); // Identify which server the user tries to reach
		message = buildResponse(server);
		write(_new_socket, message.c_str(), message.size());
		std::cout << "Response sent to " << inet_ntoa(_socketAddr.sin_addr) << " !\n" << std::endl;
		// handle_request(server);
		close(_new_socket);
		/*
		We need to find a way to stop the program properly
		If we press CTRL-C, we kill it and leaks happen
		Maybe have a web page with a dedicated button...
		(Like close_server.html, with a button sending a specific message in the client_body)

		>> if (the_stop_button_is_pressed_somewhere)
			break;
		*/
	}
	for (std::vector<int>::iterator it = _socket_list.begin(); it != _socket_list.end(); it++)
		close(*it);
}

int Webserv::newConnection(int max_fds)
{
	int new_socket = 0;

	for (int i = 3; i < max_fds; i++)
	{
		_socket = i;
		if (FD_ISSET(_socket, &_readfds))
		{
			new_socket = accept(_socket, (sockaddr *)&_socketAddr, &_socketAddrLen);
			if (new_socket != EAGAIN && new_socket != EWOULDBLOCK)
				break;
		}
	}
	std::cout << "New request received from " << inet_ntoa(_socketAddr.sin_addr) << " !\n" << std::endl;
	return new_socket;
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

std::string Webserv::buildResponse(std::string &server)
{
	std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from ";
	htmlFile.append(server);
	htmlFile.append("</p></body></html>");
    std::ostringstream ss;
    ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
       << htmlFile;

	return ss.str();
}
