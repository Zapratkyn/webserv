#include "../include/Webserv.hpp"

using namespace webserv_utils;

Webserv::Webserv()
{
}

Webserv::Webserv(const std::string &conf_file) : _conf(conf_file)
{
}

Webserv::~Webserv()
{
	for (std::vector<Server *>::const_iterator it = _server_list.begin(); it != _server_list.end(); it++)
		delete (*it);
	for (std::vector<int>::const_iterator it = _listen_socket_list.begin(); it != _listen_socket_list.end(); it++)
		close(*it);
	for (std::vector<Request>::const_iterator it = _request_list.begin(); it != _request_list.end(); it++)
		close(it->getSocket());
}


void Webserv::parseConf()
{
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

				fcntl(listen_socket, F_SETFL, O_NONBLOCK);
				_listen_socket_list.push_back(listen_socket);
				_global_socket_list.push_back(listen_socket);

				addr = *addr_it;
				if (bind(listen_socket, (sockaddr *)&addr, sizeof(addr)) < 0)
					throw bindException();

				if (listen(listen_socket, MAX_LISTEN) < 0)
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

bool run_webserv = true;

//TODO check relation signal handlers and blocking system calls
static void sigHandler(int sig_num)
{
	if (sig_num == SIGINT)
		run_webserv = false;
}

void Webserv::startListen()
{
	log("Webserv started", -1, "", 0);

	int select_return, max;
	fd_set readfds, writefds;
	fd_set read_backup, write_backup;
	struct timeval timer = {};

	FD_ZERO(&read_backup);
	FD_ZERO(&write_backup);
	FD_ZERO(&writefds);
	FD_ZERO(&readfds);
	timer.tv_sec = 1;
	timer.tv_usec = 0;
	for (std::vector<int>::iterator it = _listen_socket_list.begin(); it != _listen_socket_list.end(); it++)
		FD_SET(*it, &read_backup);

	signal(SIGINT, sigHandler);

	while (run_webserv)
	{
		errno = 0;
		FD_ZERO(&writefds);
		FD_ZERO(&readfds);
		FD_COPY(&write_backup, &writefds);
		FD_COPY(&read_backup, &readfds);
		max = *std::max_element(_global_socket_list.begin(), _global_socket_list.end());
		select_return = select(max + 1, &readfds, &writefds, NULL, &timer);
		if (errno == EINTR)
			break ;
		if (select_return < 0)
		{
			std::cerr << "Select error";
			continue;
		}
		else if (select_return == 0)
		{
			// std::cout << "Server is waiting ..." << std::endl;
			continue;
		}
		for (int i = 0; i <= max; ++i)
		{
			if (FD_ISSET(i, &readfds) && (_isListeningSocket(i)))
			{
				acceptNewConnection(i, &read_backup);
				break;
			}
			if (FD_ISSET(i, &readfds) && !_isListeningSocket(i))
			{
				readRequest(i, &read_backup, &write_backup);
				break;
			}
			if (FD_ISSET(i, &writefds) && !_isListeningSocket(i))
			{
				sendResponse(i, &read_backup, &write_backup);
				break;
			}
		}
	}
	log("Webserv stopped", -1, "", 0);
}

void Webserv::acceptNewConnection(int server_fd, fd_set *read_backup)
{
	struct sockaddr_in addr = {};
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
	_request_list.push_back(Request(new_socket, getPotentialServers(new_socket)));
	FD_SET(new_socket, read_backup);
	log("new connection", new_socket, "", 1);
}

std::vector<Server *> Webserv::getPotentialServers(int client_fd) const
{
	struct sockaddr_in addr = {};
	server_utils::getSocketAddress(client_fd, &addr);

	std::vector<Server *> potential_servers;
	std::vector<struct sockaddr_in> end_points;

	for (std::vector<Server *>::const_iterator server_it = _server_list.begin(); server_it != _server_list.end(); server_it++)
	{
		end_points = (*server_it)->getEndPoints();
		for (std::vector<struct sockaddr_in>::const_iterator end_point_it = end_points.begin();
		     end_point_it != end_points.end(); end_point_it++)
		{
			if (end_point_it->sin_addr.s_addr == addr.sin_addr.s_addr && end_point_it->sin_port == addr.sin_port)
				potential_servers.push_back(*server_it);
		}
	}
	return (potential_servers);
}

void Webserv::readRequest(int client_fd, fd_set *read_backup, fd_set *write_backup)
{
	std::vector<Request>::iterator it;
	for (it = _request_list.begin(); it != _request_list.end(); ++it)
		if (client_fd == it->_socket)
			break;
	try
	{
		bool connection = it->retrieveRequest();
		if (!connection)
		{
			FD_CLR(client_fd, read_backup);
			close(client_fd);
			log("connection closed by client", client_fd, "", 1);
			_request_list.erase(it);
			return;
		}
		else if (!it->isChunkedRequest())
		{
			FD_CLR(client_fd, read_backup);
			FD_SET(client_fd, write_backup);
		}
		log("", client_fd, it->_request_target, 2);
	}
	catch (const std::exception &e)
	{
		log(e.what(), client_fd, "", 1);
	}
}

void Webserv::sendResponse(int client_fd, fd_set *read_backup, fd_set *write_backup)
{
	std::vector<Request>::iterator it;
	for (it = _request_list.begin(); it != _request_list.end(); ++it)
		if (client_fd == it->_socket)
			break;
	try
	{
		it->_response = new Response(&*it);
		// TODO handle request based on method if _error_status is not set in the request;
		it->_response->buildMessage();
		it->_response->sendMessage();
		// TODO what with chunked requests?
		FD_CLR(client_fd, write_backup);
		log("", client_fd, it->_response->getResourcePath(), 3);
		if (it->_headers.count("Connection") &&
		    (std::find(it->_headers["Connection"].begin(), it->_headers["Connection"].end(), "keep-alive") ==
		     it->_headers["Connection"].end()))
		{
			close(client_fd);
			log("do close connection", client_fd, "", 1);
			_request_list.erase(it);
		}
		else
		{
			FD_SET(client_fd, read_backup);
			*it = Request(client_fd, it->_potential_servers);
		}
	}
	catch (const std::exception &e)
	{
		log(e.what(), client_fd, "", 1);
	}
}
