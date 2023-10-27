#include "../include/Webserv.hpp"

using namespace webserv_utils;

Webserv::Webserv(const std::string &conf_file) : _socketAddrLen(sizeof(_socketAddr)), _conf(conf_file), _kill(false)
{
	parseUrl("./www/", _url_list);
	// Uncomment to display the list of url's
	for (std::vector<std::string>::iterator it = _url_list.begin(); it != _url_list.end(); it++)
		std::cout << *it << std::endl;
	return; 
}

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
			if (!server->parseServer(server_block, server_name, port_list, _url_list))
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

			if (listen(_socket, 1000) < 0) // The second argument is the max number of connections the socket can take at a time
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

	std::cout << "Ready. Waiting for new connections...\n" << std::endl;

	while (true)
	{
		FD_ZERO(&_readfds);
		for (std::vector<int>::iterator it = _socket_list.begin(); it != _socket_list.end(); it++)
			FD_SET(*it, &_readfds);
		/*
		Select blocks until a new request is received
		It then sets the request receiving sockets to 1 and unblocks
		It means that if several requests are waiting, it will sets so many sockets to 1
		We then need to handle several potential requests in the newConnection() function
		*/
		if (select(max_fds, &_readfds, NULL, NULL, NULL) < 0)
		{
			ft_error(0);
			continue;
		}
		_new_socket = newConnection(max_fds);
		if (_new_socket > 0)
		{
			server = getServer(_server_list, _socket); // Identify which server the user tries to reach
			_server_list[server]->handleRequest(_new_socket, _socketAddr, _kill);
			close(_new_socket);
			if (_kill)
				break;
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

	Or maybe both...
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
	{
		ft_error(1);
		std::cerr << inet_ntoa(_socketAddr.sin_addr) << std::endl;
	}
	if (_previous_client.find(inet_ntoa(_socketAddr.sin_addr)) != _previous_client.end() && std::time(NULL) == _previous_client[inet_ntoa(_socketAddr.sin_addr)])
		return 0;
	_previous_client[inet_ntoa(_socketAddr.sin_addr)] = std::time(NULL);
	std::cout << "New request received from " << inet_ntoa(_socketAddr.sin_addr) << " !\n" << std::endl;
	return new_socket;
}
