#include "../include/Webserv.hpp"

using namespace webserv_utils;

Webserv::Webserv(const std::string &conf_file) : _socketAddrLen(sizeof(_socketAddr)), _conf(conf_file)
{
	_folder_list.push_back("/www/");
	_url_list.push_back("./stylesheet.css");
	_url_list.push_back("./favicon.ico");
	_url_list.push_back("./parentDirectory.png");
	_url_list.push_back("./directory.png");
	_url_list.push_back("./file.png");
	_url_list.push_back("./webPage.png");
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
	std::string				buffer, server_block;

	while(!infile.eof())
	{
		getline(infile, buffer);
		if (buffer == "server {")
		{
			server_block = getServerBlock(infile);
			Server server;
			if (!server->parseServer(server_block, _folder_list))
				throw confFailureException();
			_server_list.push_back(server);
			for(std::vector<struct sockaddr_in>::iterator it = server._end_points.begin(); it != server._end_points.end(); it++)
				_address_list.push_back(*it);
		}
	}
	infile.close();
	if (DISPLAY_SERVERS)
		displayServers(_server_list);
}	

void Webserv::startServer()
{
	std::vector<struct sockaddr_in> address_list;
	int listen_socket;

	_socketAddr.sin_family = AF_INET;

	if (!checkRedirectionList(_url_list))
		throw redirectionListException();


	/*
	Each port in the conf file is used to make an individual listening socket
	We browse the whole list, create a socket for each port
	We add the socket to its server's socket_list (for getServer()) \
	and to the global socket_list (to reset the readfds fd_set in startListen() and to close everything at the end)
	Bind() gives a "name" to each socket
	*/
	for (std::vector<Server>::iterator server_it = _server_list.begin(); server_it != _server_list.end(); server_it++)
	{
		address_list = server_it->getEndPoints();
		for (std::vector<struct sockaddr_in>::iterator addr_it = address_list.begin(); addr_it != address_list.end(); addr_it++)
		{
			if (!socketIsSet(_socket_list, *addr_it))
			{
				listen_socket = socket(AF_INET, SOCK_STREAM, 0);
				if (listen_socket < 0)
					throw openSocketException();

				fcntl(listen_socket, F_SETFL, O_NONBLOCK); // Sets the sockets to non-blocking
				_listen_socket_list.push_back(listen_socket);

				if (bind(listen_socket, (sockaddr *)addr_it, sizeof(*addr_it)) < 0)
					throw bindException();

				if (listen(listen_socket, MAX_LISTEN) < 0) // The second argument is the max number of connections the socket can take at a time
					throw listenException();
				_socket_list[listen_socket] = *addr_it;
			}
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
	int 			max_fds = _listen_socket_list.size() + 3, step = 1, select_return;
	fd_set			readfds, writefds;
	bool			kill = false;
	sigset_t		sigmask;
	
	while (!kill)
	{
		if (step == 1)
		{
			FD_ZERO(&readfds);
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
		select_return = pselect(max_fds, &readfds, &writefds, NULL, NULL, &sigmask);
		if (select_return == 0)
		{
			if (!_request_list.empty())
			{
				for (std::map<int, t_request>::iterator it = _request_list.begin(); it != _request_list.end(); it++)
				{
					it->second.url = "./www/errors/500.html";
					it->second.code = "500 Internal Server Error";
					sendText(it->second);
				}
				_request_list.clear();
			}
			step = 1;
			continue;
		}
		if (step == 1)
			acceptNewConnections(max_fds, readfds);
		else if (step == 2 && !_request_list.empty())
			readRequests(readfds, writefds);
		else if (step == 3 && !_request_list.empty())
			sendRequests(kill, writefds, max_fds);
		if (++step == 4)
			step = 1;
	}
	FD_ZERO(&readfds);
	FD_ZERO(&writefds);
	log("Webserv stopped", "", "", "", 0);
}

void Webserv::acceptNewConnections(int &max_fds, fd_set &readfds)
{
	int new_socket;
	struct t_request new_request;
	std::vector<int> new_socket_list;

	for (int socket = 3; socket < max_fds; socket++)
	{
		if (FD_ISSET(socket, &readfds))
		{
			while (true)
			{
				initRequest(new_request);
				new_socket = accept(socket, (sockaddr *)&_socketAddr, &_socketAddrLen);
				if (new_socket < 0)
					break;
				new_request.potentialServers = getPotentialServers(_server_list, _socket_list[socket], new_request);
				new_request.client = _socketAddr.sin_addr;
				new_request.socket = new_socket;
				_request_list.push_back(new_request);
				max_fds++;
				new_socket_list.push_back(new_socket);
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
	for (std::vector<struct t_request>::iterator it = _request_list.begin(); it != _request_list.end(); it++)
	{
		if (FD_ISSET(it->socket, &readfds))
		{
			try
			{
				getRequest(*it);
				if (it->host == "")
				{
					it->code = "400 Bad Request";
					it->url = "./www/errors/400.html";
					sendText(it);
				}
				getServer(it->potentialServers, it->host);
				FD_SET(it->socket, &writefds);
			}
			catch(const std::exception& e)
			{
				log(e.what(), it->client, "", "", 1);
			}
		}
	}
	// We don't need to read the requesting sockets anymore
	FD_ZERO(&readfds);
}

void Webserv::sendRequests(bool &kill, fd_set &writefds, int &max_fds)
{
	for (std::vector<struct t_request>::iterator it = _request_list.begin(); it != _request_list.end();)
	{
		if (FD_ISSET(it->socket, &writefds))
		{
			it->server->handleRequest(it, _url_list, kill);
			if (kill)
				break;
			close(it->socket); // Closes the socket so it can be used again later
			it = _request_list.erase(it);
			max_fds--;
		}
	}
	// We don't want select() to test this fd_set anymore
	FD_ZERO(&writefds);
	_request_list.clear();
}
