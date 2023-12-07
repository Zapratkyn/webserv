#include "../include/Webserv.hpp"

using namespace webserv_utils;

Webserv::Webserv(const std::string &conf_file) : _conf(conf_file)
{
	return;
}

Webserv::~Webserv()
{
	for (std::vector<Server *>::iterator it = _server_list.begin(); it != _server_list.end(); it++)
		delete (*it);
	for (std::vector<int>::iterator it = _listen_socket_list.begin(); it != _listen_socket_list.end(); it++)
		close(*it);
	for (std::vector<struct t_request>::iterator it = _request_list.begin(); it != _request_list.end(); it++)
		close(it->socket);
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
	std::ifstream infile(_conf.c_str());
	std::string buffer, server_block;
	Server *server;

	while (!infile.eof())
	{
		getline(infile, buffer);
		if (buffer == "server {")
		{
			server_block = getServerBlock(infile);
			server = new Server;
			if (!server->parseServer(server_block))
			{
				delete server;
				throw confFailureException();
			}
			_server_list.push_back(server);
		}
	}
	infile.close();
	if (DISPLAY_SERVERS)
		displayServers(_server_list);
}

void Webserv::startServer()
{
	std::vector<struct sockaddr_in> address_list;
	struct sockaddr_in addr;
	int listen_socket;
	int reuse = true;

	//	if (!checkRedirectionList(_url_list))
	//		throw redirectionListException();

	/*
	Each port in the conf file is used to make an individual listening socket
	We browse the whole list, create a socket for each port
	We add the socket to its server's socket_list (for getServer()) \
	and to the global socket_list (to reset the readfds fd_set in startListen() and to close everything at the end)
	Bind() gives a "name" to each socket
	*/
	for (std::vector<Server *>::iterator server_it = _server_list.begin(); server_it != _server_list.end(); server_it++)
	{
		address_list = (*server_it)->getEndPoints();
		for (std::vector<struct sockaddr_in>::iterator addr_it = address_list.begin(); addr_it != address_list.end();
		     addr_it++)
		{
			if (!socketIsSet(_socket_list, *addr_it))
			{
				listen_socket = socket(AF_INET, SOCK_STREAM, 0);
				if (listen_socket < 0)
					throw openSocketException();

				if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
					throw setSocketoptionException();

				fcntl(listen_socket, F_SETFL, O_NONBLOCK); // Sets the sockets to non-blocking
				_listen_socket_list.push_back(listen_socket);
				_global_socket_list.push_back(listen_socket);

				addr = *addr_it;
				if (bind(listen_socket, (sockaddr *)&addr, sizeof(addr)) < 0)
					throw bindException();

				if (listen(listen_socket, MAX_LISTEN) <
				    0) // The second argument is the max number of connections the socket can take at a time
					throw listenException();
				_socket_list[listen_socket] = *addr_it;
			}
		}
	}
}

bool Webserv::_isListeningSocket(int fd)
{
	std::vector<int>::const_iterator it = find(_listen_socket_list.begin(), _listen_socket_list.end(), fd);
	return (it != _listen_socket_list.end());
}

void Webserv::startListen()
{
	std::cout << "\n\n### Webserv started ###\n\n" << std::endl;
	log("Webserv started", "", "", 0);

	/*
	Select() needs the biggest fd + 1 from all the fd_sets
	Since fd 0, 1 and 2 are already taken (STD_IN and STD_OUT and STD_ERR), our list begins at 3
	Therefore, max_fds = total_number_of_sockets + STD_IN + STD_OUT + STD_ERR
	*/
	int select_return, max;
	fd_set readfds, writefds;
	bool kill = false;
	fd_set read_backup, write_backup;
	struct timeval timer = {};

	FD_ZERO(&read_backup);
	FD_ZERO(&write_backup);
	timer.tv_sec = 2;
	timer.tv_usec = 0;
	for (std::vector<int>::iterator it = _listen_socket_list.begin(); it != _listen_socket_list.end(); it++)
		FD_SET(*it, &read_backup);
	while (!kill)
	{
		FD_ZERO(&writefds);
		FD_ZERO(&readfds);
		FD_COPY(&write_backup, &writefds);
		FD_COPY(&read_backup, &readfds);
		max = *std::max_element(_global_socket_list.begin(), _global_socket_list.end());
		select_return = select(max + 1, &readfds, &writefds, NULL, &timer);
		if (select_return < 0)
		{
			std::cerr << "Select error" << std::endl;
			continue;
		}
		else if (select_return == 0)
		{
			std::cout << "Server is waiting ..." << std::endl;
			continue;
		}
		for (int i = 0; i <= max; ++i)
		{
			if (FD_ISSET(i, &readfds) && (_isListeningSocket(i)))
			{
				acceptNewConnections(i, &read_backup);
				break ;
			}
			if (FD_ISSET(i, &readfds) && !_isListeningSocket(i))
			{
				readRequests(i, &read_backup, &write_backup);
				break ;
			}
			if (FD_ISSET(i, &writefds) && !_isListeningSocket(i))
			{
				sendRequests(i, kill, &read_backup, &write_backup);
				break ;
			}
		}
	}
	log("Webserv stopped", "", "", 0);
}

void Webserv::acceptNewConnections(int server_fd, fd_set *read_backup)
{
	struct sockaddr_in addr = _socket_list[server_fd];
	socklen_t addr_len = sizeof(addr);

	int new_socket = accept(server_fd, (sockaddr *)&addr, &addr_len);
	if (new_socket < 0)
	{
		std::cerr << "Error : Failed to create socket for new connection" << std::endl;
		return;
	}
	fcntl(new_socket, F_SETFL, O_NONBLOCK);
	int reuse = 1;
	setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));
	_global_socket_list.push_back(new_socket);
	FD_SET(new_socket, read_backup);
}

static int getSocketAddress(int socket, struct sockaddr_in *addr)
{
	socklen_t len = sizeof *addr;
	return (getsockname(socket, (struct sockaddr *)addr, &len));
}

void Webserv::readRequests(int client_fd, fd_set *read_backup, fd_set *write_backup)
{
	bool keep_alive;
	struct t_request new_request = {};
	struct sockaddr_in addr = {};
	(void)write_backup;

	// THIS is the address of the server that init the connection with the client
	getSocketAddress(client_fd, &addr);

	initRequest(new_request);
	getPotentialServers(_server_list, addr, new_request);
	new_request.socket = client_fd;

	try
	{
		keep_alive = getRequest(new_request);
		if (!keep_alive)
		{
			FD_CLR(client_fd, read_backup);
			close(client_fd);
		}
		else
		{
			getServer(new_request);
			FD_CLR(client_fd, read_backup);
			FD_SET(client_fd, write_backup);
			_request_list.push_back(new_request);
		}
	}
	catch (const std::exception &e)
	{
		log(e.what(), new_request.client, "", 1);
	}
}

static const std::string &getConnectionHeader(struct t_request &request)
{
	std::map<std::string, std::vector<std::string> >::iterator it;
	it = request.headers.find("Connection");
	return (it->second[0]);
}

void Webserv::sendRequests(int client_fd, bool &kill, fd_set *read_backup, fd_set *write_backup)
{
	std::vector<struct t_request>::iterator it;
	for (it = _request_list.begin(); it != _request_list.end(); ++it)
		if (client_fd == it->socket)
			break;
	if (it == _request_list.end())
	{
		std::cerr << "For some reason, we can't find your request" << std::endl;
		return;
	}
	it->server->handleRequest(*it, kill);
	FD_CLR(client_fd, write_backup);
	if (getConnectionHeader(*it) == "close")
		close(client_fd);
	else
		FD_SET(client_fd, read_backup);
	_request_list.erase(it);
}
