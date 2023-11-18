#include "../include/Webserv.hpp"

using namespace webserv_utils;

Webserv::Webserv(const std::string &conf_file) : _socketAddrLen(sizeof(_socketAddr)), _conf(conf_file)
{
	_folder_list.push_back("/www/");
	_url_list.push_back("./stylesheet.css");
	_url_list.push_back("./favicon.ico");
	parseUrl("./www/", _url_list, _folder_list);
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
			if (!server->parseServer(server_block, server_name, port_list, _folder_list))
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
	std::vector<int> port_list, full_port_list;
	int listen_socket;

	initSockaddr(_socketAddr);
	initTimeval(_tv);


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
			full_port_list.push_back(*port_it);
			listen_socket = socket(AF_INET, SOCK_STREAM, 0);
			if (listen_socket < 0)
				throw openSocketException();
			
			fcntl(listen_socket, F_SETFL, O_NONBLOCK); // Sets the sockets to non-blocking
			_listen_socket_list.push_back(listen_socket);
			server_it->second->addSocket(listen_socket);

			_socketAddr.sin_port = htons(*port_it); // Link the socket to the corresponding port
			if (bind(listen_socket, (sockaddr *)&_socketAddr, _socketAddrLen) < 0)
				throw bindException();

			if (listen(listen_socket, MAX_LISTEN + 1) < 0) // The second argument is the max number of connections the socket can take at a time
				throw listenException();
		}
	}
}

void Webserv::startListen()
{
	listenLog(_socketAddr, _server_list); // Displays all the open ports to the user on the terminal
	log("Webserv started", "", "", "", 0);

	/*
	Select() needs the biggest fd + 1 from all the fd_sets
	Since fd 0, 1 and 2 are already taken (STD_IN and STD_OUT and STD_ERR), our list begins at 3
	Therefore, max_fds = total_number_of_sockets + STD_IN + STD_OUT + STD_ERR
	*/
	int 			max_fds = _listen_socket_list.size() + 3, step = 1;
	fd_set			readfds, writefds;
	bool			kill = false;
	
	while (!kill)
	{
		if (step == 1)
		{
			for (std::vector<int>::iterator it = _listen_socket_list.begin(); it != _listen_socket_list.end(); it++)
				FD_SET(*it, &readfds);
		}
		/*
		Select blocks until a new request is received
		It then sets the request receiving sockets to 1 and unblocks
		acceptNewConnections() will create new sockets and add them to the readfds fd_set
		After a 2nd select(), we read from the newly created sockets and parse the requests' headers and bodies
		Go through select() again before handling the stacked requests
		Then handle all the stacked requests
		Reset the readfds with the listening sockets (see above)
		Go through the whole process again
		*/
		if (select(max_fds, &readfds, &writefds, NULL, NULL) < 0)
		{
			ft_error(0);
			continue;
		}
		if (step == 1)
			acceptNewConnections(max_fds, readfds);
		else if (step == 2 && !_request_list.empty())
			readRequests(readfds, writefds);
		else if (step == 3 && !_request_list.empty())
			sendRequests(kill, writefds);
		if (++step == 4)
			step = 1;
	}
	log("Webserv stopped", "", "", "", 0);
}

void Webserv::acceptNewConnections(int &max_fds, fd_set &readfds)
{
	int new_socket, listen;
	struct t_request new_request;
	std::vector<int> new_socket_list;

	for (int socket = 3; socket < max_fds; socket++)
	{
		if (FD_ISSET(socket, &readfds))
		{
			listen = 1;
			while (true)
			{
				initRequest(new_request);
				new_socket = accept(socket, (sockaddr *)&_socketAddr, &_socketAddrLen);
				if (new_socket < 0)
					break;
				if (listen > MAX_LISTEN)
				{
					new_request.url = "./www/errors/500.html";
					new_request.code = "500 Internal Server Error";
					sendText(new_request);
					close(new_socket);
					continue;
				}
				new_request.server = getServer(_server_list, socket);
				new_request.client = inet_ntoa(_socketAddr.sin_addr);
				new_request.socket = new_socket;
				_request_list[new_socket] = new_request;
				max_fds++;
				new_socket_list.push_back(new_socket);
				listen++;
			}
		}
	}
	// We keep only the new sockets to avoid having select() applied on the same sockets several times
	FD_ZERO(&readfds);
	for (std::vector<int>::iterator it = new_socket_list.begin(); it != new_socket_list.end(); it++)
		FD_SET(*it, &readfds);
}

void Webserv::readRequests(fd_set &readfds, fd_set &writefds)
{
	for (std::map<int, t_request>::iterator it = _request_list.begin(); it != _request_list.end(); it++)
	{
		if (FD_ISSET(it->first, &readfds))
		{
			try
			{
				getRequest(_server_list[it->second.server]->getBodySize(), it->second);
				FD_SET(it->first, &writefds);
			}
			catch(const std::exception& e)
			{
				log(e.what(), it->second.client, "", "", 1);
			}
		}
	}
	// We don't need to read the requesting sockets anymore
	FD_ZERO(&readfds);
}

void Webserv::sendRequests(bool &kill, fd_set &writefds)
{
	std::map<int, struct t_request>::iterator tmp;

	for (std::map<int, struct t_request>::iterator it = _request_list.begin(); it != _request_list.end();)
	{
		if (FD_ISSET(it->first, &writefds))
		{
			_server_list[it->second.server]->handleRequest(it->second, _url_list, kill);
			if (kill)
				break;
			close(it->first); // Closes the socket so it can be used again later
			tmp = it++; // If I erase an iterator while itering on a std::map, I get a SEGFAULT
			_request_list.erase(tmp);
		}
	}
	// We don't want select() to test this fd_set anymore
	FD_ZERO(&writefds);
	_request_list.clear();
}
