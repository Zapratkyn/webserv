#include "../include/Webserv.hpp"

using namespace webserv_utils;

Webserv::Webserv(const std::string &conf_file) : _socketAddrLen(sizeof(_socketAddr)), _conf(conf_file) { return; }

Webserv::~Webserv() 
{
	// Since servers in the server list are dynamically allocated pointers, we delete each server one by one in the destructor at the end of the program
	for (std::map<std::string, Server*>::iterator it = _server_list.begin(); it != _server_list.end(); it++)
		delete it->second;
	// I thought about having servers as variables instead of pointers, to get rid of the need of deleting them but...
	// We need to close all the sockets opened by the socket() function
	for (std::vector<int>::iterator it = _socket_list.begin(); it != _socket_list.end(); it++)
		close(*it);
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
	// Uncomment to display parsed servers
	// displayServers(_server_list);
}

void Webserv::startServer()
{
	std::vector<int> port_list;

	initSockaddr(_socketAddr);
	// initTimeval(_tv);


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
			
			_socket_list.push_back(_socket);
			server_it->second->addSocket(_socket);

			_socketAddr.sin_port = htons(*port_it);
			if (bind(_socket, (sockaddr *)&_socketAddr, _socketAddrLen) < 0)
				throw bindException();

			if (listen(_socket, 1000) < 0)
				throw listenException();
		}
	}
}

void Webserv::startListen()
{
	listenLog(_socketAddr, _server_list); // Displays all the open ports to the user

	/*
	Select() needs the biggest fd + 1 from all the fd_sets
	Since fd 0, 1 and 2 are already taken (STD_IN and STD_OUT and STD_ERR), our list begins at 3
	Therefore, max_fds = total_number_of_sockets + STD_IN + STD_OUT + STD_ERR
	*/
	int max_fds = _socket_list.size() + 3;
	std::string server;
	std::string message;

	while (true)
	{
		FD_ZERO(&_readfds);
		for (std::vector<int>::iterator it = _socket_list.begin(); it != _socket_list.end(); it++)
			FD_SET(*it, &_readfds);
		std::cout << "Waiting for a new connection...\n" << std::endl;
		if (select(max_fds, &_readfds, NULL, NULL, NULL) < 0) // Blocks until a new request is received
		{
			ft_error(0, _socketAddr);
			usleep(10000); // To delete once the response handling is OK. Prevents multi requests from the same client
			continue;
		}
		_new_socket = newConnection(max_fds);
		if (_new_socket > 0)
		{
			server = getServer(_server_list, _socket); // Identify which server the user tries to reach
			if (getRequest(_server_list[server]->getBodySize())) // Separate request's header and body (if any)
			{
				message = buildResponse(server);
				write(_new_socket, message.c_str(), message.size());
				std::cout << "Response sent to " << inet_ntoa(_socketAddr.sin_addr) << " !\n" << std::endl;
				// try
				// {
				// 	_server_list[server]->handle_request(_request_header, _request_body, _new_socket);
				// }
				// catch(const std::exception& e)
				// {
				// 	std::cerr << e.what() << '\n';
				// }
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
		}
	}
}

int Webserv::newConnection(int max_fds)
{
	/*
	At some point, we will probably need to stack requests
	To do that, we can :
	- Use the _readfds fd_set to add/remove incoming/handled requests
	or
	- Use threads to let webserv handle several requests at a time
	*/
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
	if (_new_socket < 0)
		ft_error(1, _socketAddr);
	std::cout << "New request received from " << inet_ntoa(_socketAddr.sin_addr) << " !\n" << std::endl;
	return new_socket;
}

bool Webserv::getRequest(size_t bodySize)
{
	int bytesReceived;
	char buffer[100000] = {0};

	bytesReceived = read(_new_socket, buffer, 100000);
	if (bytesReceived < 0)
	{
		ft_error(2, _socketAddr);
		return false;
	}

	std::string oBuffer(buffer);
	std::stringstream ifs(oBuffer);

	_request_header = "";
	_request_body = "";

	while (!ifs.eof() && oBuffer.size())
	{
		getline(ifs, oBuffer);
		// Uncomment to display the request header
		// std::cout << oBuffer << std::endl;
		if (oBuffer.size())
			_request_header.append(oBuffer);
	}
	while (!ifs.eof())
	{
		getline(ifs, oBuffer);
		// Uncomment to display the request body (if any)
		// std::cout << oBuffer << std::endl;
		_request_body.append(oBuffer);
	}
	if (_request_body.size() > bodySize)
	{
		ft_error(3, _socketAddr);
		return false;
	}
	// std::cout << _request_header << "\n" << _request_body << std::endl;
	return true;
}

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
