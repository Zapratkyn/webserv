#include "../include/Webserv.hpp"

using namespace webserv_utils;

Webserv::Webserv(const std::string &conf_file) : _socketAddrLen(sizeof(_socketAddr)), _conf(conf_file)
{
	// _log_file.open("./webserv.log", std::ofstream::app);
	parseUrl("./www/", _url_list);
	if (DISPLAY_URL)
	{
		for (std::vector<std::string>::iterator it = _url_list.begin(); it != _url_list.end(); it++)
			std::cout << *it << std::endl;
	}
	return; 
}

Webserv::~Webserv() 
{
	// Since servers in the server list are dynamically allocated pointers, we delete each server one by one in the destructor at the end of the program
	for (std::map<std::string, Server*>::iterator it = _server_list.begin(); it != _server_list.end(); it++)
		delete it->second;
	// I thought about having servers as variables instead of pointers, to get rid of the need of deleting them but...
	// We need to close all the sockets opened by the socket() function
	for (std::vector<int>::iterator it = _listen_socket_list.begin(); it != _listen_socket_list.end(); it++)
		close(*it);
	for (std::map<int, struct t_request>::iterator it = _request_list.begin(); it != _request_list.end(); it++)
		close(it->first);
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
	if (DISPLAY_SERVERS)
		displayServers(_server_list);
}	

void Webserv::startServer()
{
	std::vector<int> port_list;
	int listen_socket;

	initSockaddr(_socketAddr);
	// initTimeval(_tv);


	/*
	Each port in the conf file is used to make an individual listening socket
	We browse the whole list, create a socket for each port
	We add the socket to its server's socket_list (for getServer()) \
	and to the global socket_list (to reset the readfds fd_set in startListen() and to close everything at the end)
	Bind() gives a "name" to each socket
	*/
	for (std::map<std::string, Server*>::iterator server_it = _server_list.begin(); server_it != _server_list.end(); server_it++)
	{
		port_list = server_it->second->getPorts();
		for (std::vector<int>::iterator port_it = port_list.begin(); port_it != port_list.end(); port_it++)
		{
			listen_socket = socket(AF_INET, SOCK_STREAM, 0);
			if (listen_socket < 0)
				throw openSocketException();
			
			fcntl(listen_socket, F_SETFL, O_NONBLOCK); // Sets the sockets to non-blocking
			_listen_socket_list.push_back(listen_socket);
			server_it->second->addSocket(listen_socket);

			_socketAddr.sin_port = htons(*port_it); // Link the socket to the corresponding port
			if (bind(listen_socket, (sockaddr *)&_socketAddr, _socketAddrLen) < 0)
				throw bindException();

			if (listen(listen_socket, MAX_LISTEN) < 0) // The second argument is the max number of connections the socket can take at a time
				throw listenException();
		}
	}
}

void Webserv::startListen()
{
	listenLog(_socketAddr, _server_list); // Displays all the open ports to the user
	log("Webserv started", "", "", "", 0);

	/*
	Select() needs the biggest fd + 1 from all the fd_sets
	Since fd 0, 1 and 2 are already taken (STD_IN and STD_OUT and STD_ERR), our list begins at 3
	Therefore, max_fds = total_number_of_sockets + STD_IN + STD_OUT + STD_ERR
	*/
	int 			max_fds = _listen_socket_list.size() + 3;
	fd_set			readfds, writefds;
	std::string 	server;
	std::map<int, struct t_request>::iterator tmp;
	bool			kill = false;

	FD_ZERO(&writefds);
	
	while (true)
	{
		FD_ZERO(&readfds); // Reset the readfds fd_set
		for (std::vector<int>::iterator it = _listen_socket_list.begin(); it != _listen_socket_list.end(); it++)
			FD_SET(*it, &readfds);
		/*
		Select blocks until a new request is received
		It then sets the request receiving sockets to 1 and unblocks
		We then need to parse several potential requests in the newConnection() function
		This will first stack pending requests (in the writefds fd_set and in the _request_list) \
		Go through select() before handling the stacked requests \
		Then handle all the stacked requests and go back to select() for another round
		*/
		if (select(max_fds + _request_list.size(), &readfds, &writefds, NULL, NULL) < 0)
		{
			ft_error(0);
			continue;
		}
		if (_request_list.empty())
		{
			if (!acceptNewConnections(max_fds, readfds, writefds)) // Is a bool to allow us to shut the program down properly
				break;
		}
		else
		{
			for (std::map<int, struct t_request>::iterator it = _request_list.begin(); it != _request_list.end();)
			{
				if (FD_ISSET(it->first, &writefds)) // Works only if select() said so
				{
					_server_list[it->second.server]->handleRequest(it->second, _url_list, kill);
					if (kill)
						break;
					close(it->first); // Closes the socket so it can be used again later
					FD_CLR(it->first, &writefds); // Clears the writefds fd_set
					tmp = it; // If I erase an iterator while itering on a std::map, I get a SEGFAULT
					it++;
					_request_list.erase(tmp);
				}
			}
		}
		if (kill)
			break;
	}
	log("Webserv stopped", "", "", "", 0);
}

bool Webserv::acceptNewConnections(int max_fds, fd_set &readfds, fd_set &writefds)
{
	int time, new_socket;
	struct t_request new_request;

	for (int socket = 3; socket < max_fds; socket++)
	{
		if (FD_ISSET(socket, &readfds))
		{
			/*
			One socket may have up to MAX_LISTEN pending requests
			So we use accept() on each socket until all pending requests have been parsed
			*/
			while(true)
			{
				initRequest(new_request);
				new_request.server = getServer(_server_list, socket);
				new_socket = accept(socket, (sockaddr *)&_socketAddr, &_socketAddrLen);
				if (new_socket == EAGAIN || new_socket == EWOULDBLOCK)
					break; // If no more pending request
				new_request.client = inet_ntoa(_socketAddr.sin_addr);
				time = std::time(NULL);
				if (_previous_clients.find(new_request.client) != _previous_clients.end() \
					&& (_previous_clients[new_request.client] == time \
					|| _previous_clients[new_request.client] == time - 1))
				{
					// If the same client has already sent a request in the last or current second
					close(new_socket); // It will not be used this time, so we close it and use it again later 
					break;
				}
				else
				{
					_previous_clients[new_request.client] = std::time(NULL);
					FD_SET(new_socket, &writefds); // We add the new_socket to the writefds fd_set
					new_request.socket = new_socket;
					try
					{
						getRequest(_server_list[new_request.server]->getBodySize(), new_request);
						_request_list[new_socket] = new_request;
					}
					catch(const std::exception& e)
					{
						close(new_socket);
						log(e.what(), new_request.client, "", "", 1);
					}
				}
			}
		}
	}
	return true;
}
