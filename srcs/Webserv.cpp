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
	for (std::vector<Server *>::iterator it = _server_list.begin(); it != _server_list.end(); it++)
		delete (*it);
	for (std::vector<int>::iterator it = _listen_socket_list.begin(); it != _listen_socket_list.end(); it++)
		close(*it);
	for (std::vector<Request>::iterator it = _request_list.begin(); it != _request_list.end(); it++)
		close(it->getSocket());
	return;
}

void Webserv::setConfigFile(const std::string &conf_file)
{
	_conf = conf_file;
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
	log("Webserv started", -1, "", 0);

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
	timer.tv_sec = 1;
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
				sendResponse(i, kill, &read_backup, &write_backup);
				break;
			}
		}
	}
	log("Webserv stopped", -1, "", 0);
}

void Webserv::acceptNewConnection(int server_fd, fd_set *read_backup)
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

	_request_list.push_back(Request(new_socket));

	FD_SET(new_socket, read_backup);
	log("new connection", new_socket, "", 1);
}

void Webserv::setServerForRequest(Request &request)
{
	struct sockaddr_in addr = {};
	server_utils::getSocketAddress(request.getSocket(), &addr);

	std::vector<Server *> potentialServers;
	std::vector<struct sockaddr_in> end_points;

	for (std::vector<Server *>::iterator server_it = _server_list.begin(); server_it != _server_list.end(); server_it++)
	{
		end_points = (*server_it)->getEndPoints();
		for (std::vector<struct sockaddr_in>::iterator end_point_it = end_points.begin();
		     end_point_it != end_points.end(); end_point_it++)
		{
			if (end_point_it->sin_addr.s_addr == addr.sin_addr.s_addr && end_point_it->sin_port == addr.sin_port)
				potentialServers.push_back(*server_it);
		}
	}

	Server *server = *potentialServers.begin();
	request._server = server;

	std::vector<std::string> host;
	if (request.getValueOfHeader("Host", host) && !host.empty())
	{
		std::string host_name = host[0];

		for (std::vector<Server *>::iterator server_it = potentialServers.begin(); server_it != potentialServers.end();
		     server_it++)
		{
			std::vector<std::string> names = (*server_it)->getServerNames();
			for (std::vector<std::string>::iterator name_it = names.begin(); name_it != names.end(); name_it++)
			{
				if (*name_it == host_name)
					request._server = *server_it;
			}
		}
	}
	if (DISPLAY_SERVER_FOR_REQUEST)
	{
		int i = 0;
		for (std::vector<Server *>::iterator server_it = _server_list.begin(); server_it != _server_list.end();
		     server_it++)
		{
			if (request._server == *server_it)
				break;
			i++;
		}
		std::cout << "*****************************************" << std::endl;
		std::cout << "Server handling request : server #" << i << std::endl;
		std::cout << "*****************************************" << std::endl << std::endl;
	}
}

void Webserv::setLocationForRequest(Request &request)
{
	std::string best_match("/");
	size_t size_best_match = best_match.size();

	UrlParser url_parsed(request.getRequestTarget());

	std::map<std::string, t_location>::const_iterator cit;
	for (cit = request._server->getLocationlist().begin(); cit != request._server->getLocationlist().end(); ++cit)
	{
		size_t pos = url_parsed.path.find(cit->first);
		if (pos == 0 && cit->first.size() > size_best_match)
			best_match = cit->first;
	}
	request._server_location = best_match;
	if (DISPLAY_LOCATION_FOR_REQUEST)
	{
		std::cout << "************** Url Parser ***************" << std::endl;
		std::cout << "[ UrlParser for " <<  url_parsed.resource << " ]" << std::endl;
		std::cout << url_parsed;
		std::cout << "best match for request_location is " << best_match << std::endl;
		std::cout << "*****************************************" << std::endl << std::endl;
	}
}

void Webserv::checkMaxBodySize(Request &request)
{
	if (request._content_length > static_cast<size_t>(request._server->getBodySize()))
		request._error_status = 413;
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
			_request_list.erase(it);
			return;
		}
		else if (!it->isChunkedRequest())
		{
			setServerForRequest(*it); //TODO move setPotentialServers() to AcceptNewConnection()
			                          // move setFinalServer() to retrieveRequest()
			setLocationForRequest(*it); //TODO move to retrieveRequest()

			if (!it->_error_status)
				checkMaxBodySize(*it); //TODO is this set for a server block or also for a location?
			FD_CLR(client_fd, read_backup);
			FD_SET(client_fd, write_backup);
		}
	}
	catch (const std::exception &e)
	{
		log(e.what(), client_fd, "", 1);
	}
}

void Webserv::sendResponse(int client_fd, bool &kill, fd_set *read_backup, fd_set *write_backup)
{
	(void)kill;

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
		if (it->_headers.count("Connection") &&
		    (std::find(it->_headers["Connection"].begin(), it->_headers["Connection"].end(), "keep-alive") ==
		     it->_headers["Connection"].end()))
		{
			close(client_fd);
			log("closed connection", client_fd, "", 1);
			_request_list.erase(it);
		}
		else
		{
			FD_SET(client_fd, read_backup);
			it->_resetRequest();
		}
	}
	catch (const std::exception &e)
	{
		log(e.what(), client_fd, "", 1);
	}
}
