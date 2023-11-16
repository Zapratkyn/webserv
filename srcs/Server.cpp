#include "../include/Server.hpp"

using namespace server_utils;

Server::Server() : _host(""), _server_name(""), _root(""), _index(""), _client_max_body_size(-1) { return; }
Server::~Server() { return; }

bool Server::setHost(const std::string &host)
{
	if (_host != "")
	{
		ft_error(0, host, "host");
		return false;
	}
	_host = host;
	return true;
}
bool Server::setServerName(const std::string &name)
{
	if (_server_name != "")
	{
		ft_error(0, name, "server_name");
		return false;
	}
	_server_name = name;
	return true;
}
bool Server::setRoot(std::string &root)
{
	std::string slash = "/";

	if (_root != "")
	{
		ft_error(0, root, "root");
		return false;
	}
	// We need root path to start and end with '/'
	if (root[0] != '/')
		root = slash.append(root);
	if (root[root.size() - 1] != '/')
		root.append("/");
	if (root == "/")
		root = "/www/";
	_root = root;
	return true;
}
bool Server::setBodySize(const std::string &size)
{
	if (_client_max_body_size >= 0 || size.find_first_not_of(DIGITS) != size.npos)
	{
		ft_error(2, size, "client_max_body_size");
		return false;
	}
	_client_max_body_size = ft_stoi(size);
	return true;
}
bool Server::setIndex(const std::string &index)
{
	if (_index != "")
	{
		ft_error(0, index, "index");
		return false;
	}
	_index = index;
	return true;
}
bool Server::addPort(const std::string &value, std::vector<int> &port_list)
{
	int iValue;

	if (value.find_first_not_of(DIGITS) != value.npos)
	{
		ft_error(2, value, "port");
		return false;
	}
	iValue = ft_stoi(value);
	if (!port_list.empty())
	{
		for (std::vector<int>::iterator it = port_list.begin(); it != port_list.end(); it++)
		{
			if (*it == iValue)
			{
				ft_error(0, value, "port");
				return false;
			}
		}
	}
	_ports.push_back(iValue);
	port_list.push_back(iValue);
	return true;
}
bool Server::addLocation(std::stringstream &ifs, std::string &value)
{
	std::string location_block, slash = "/";

	if (!_location_list.empty())
	{
		if (_location_list.find(value) != _location_list.end())
		{
			ft_error(0, value, "location");
			return false;
		}
	}
	location_block = getLocationBlock(ifs);
	if (location_block.size())
	{
		value = slash.append(value);
		_location_list[value] = newLocation(value, location_block);
		if (!_location_list[value].valid)
			return false;
	}
	return true;
}
void Server::addSocket(int &socket)
{
	_sockets.push_back(socket);
	return;
}
void Server::addDefaultLocation()
{
	t_location default_location;

	if (_index != "")
		default_location.index = _index;
	else
		default_location.index = "pages/index.html";
	if (_root != "")
		default_location.root = _root;
	else
		default_location.root = "/www/";
	default_location.location = "/";
	default_location.autoindex = "on";
	default_location.methods.push_back("GET");
	
	_location_list["/"] = default_location;
}


std::string							Server::getHost() const { return _host; }
std::string 						Server::getServerName() const { return _server_name; }
std::string 						Server::getRoot() const { return _root; }
std::string 						Server::getIndex() const { return _index; }
int 								Server::getBodySize() const { return _client_max_body_size; }
std::vector<int> 					Server::getPorts() const { return _ports; }
std::map<std::string, t_location> 	Server::getLocationlist() const { return _location_list; }
std::vector<int>					Server::getSockets() const { return _sockets; }

/*
In the 2 functions below :
We use the server_block to parse any option written in it to the server's attributes
If an option doesn't exist or is in double, we throw an error and stop the program
If a line doesn't start with "location" and doesn't end with a ';' or a bracket, we throw an error and stop the program
There can be several ports and locations (structures) in a server
There can be several methods in a location
Location names are used to make sure a same location is not used more than once
*/
bool Server::parseOption(const int &option, std::string &value, std::stringstream &ifs, const std::string &server_name, std::vector<int> &port_list)
{
	switch (option) {
		case 0:
			if (!addPort(value, port_list))
				return false;
			break;
		case 1:
			if (!setHost(value))
				return false;
			break;
		case 2:
			if (!setServerName(server_name))
				return false;
			break;
		case 3:
			if (!setBodySize(value))
				return false;
			break;
		case 4:
			if (!setRoot(value))
				return false;
			break;
		case 5:
			if (!setIndex(value))
				return false;
			break;
		case 6:
			if (!addLocation(ifs, value))
				return false;
			break;
	}
	return true;
}

bool Server::parseServer(const std::string &server_block, const std::string &server_name, std::vector<int> &port_list, std::vector<std::string> &folder_list)
{
	std::string 		buffer, name, value, option_list[7] = {"listen", "host", "server_name", "client_max_body_size", "root", "index", "location"};
	std::stringstream	ifs(server_block); // std::stringstream works the same as a std::ifstream but is constructed from a string instead of a file
	int					option;
	t_location			folder;
	
	while (!ifs.eof())
	{
		getline(ifs, buffer);
		if (!buffer.size()) // Skips empty lines
			continue;
		name = getOptionName(buffer);
		value = getOptionValue(buffer);
		if (buffer[buffer.size() - 1] != ';')
		{
			if (buffer.substr(0, buffer.find_first_of(" \t")) != "location")
			{
				ft_error(1, value, name);
				return false;
			}
		}
		for (int i = 0; i <= 7; i++)
		{
			option = i;
			if (name == option_list[i])
				break;
		}
		/*
		Thanks to the 'option == 7' condition, we don't need a default 
		behavior for the switch statement in the parseOption() function
		*/
		if (option == 7)
		{
			ft_error(4, name, "");
			return false;
		}
		if (!parseOption(option, value, ifs, server_name, port_list))
			return false;
	}
	if (_ports.empty())
	{
		std::cerr << server_name << " needs at least one port" << std::endl;
		return false;
	}
	if (_client_max_body_size == -1)
		_client_max_body_size = 60000; // The PDF states we need to limit the client_max_body_size
	if (_location_list.find("/") == _location_list.end())
		addDefaultLocation();
	for (std::vector<std::string>::iterator it = folder_list.begin(); it != folder_list.end(); it++)
	{
		folder.autoindex = "on";
		folder.root = *it;
		folder.location = *it;
		_location_list[*it] = folder;
	}
	return true;
}

void Server::handleRequest(struct t_request &request, std::vector<std::string> &url_list, bool &kill)
{
	try
	{
		setRequest(request, kill); // Gets the method and the location from the request
		if (!kill)
		{
			/*
			Truncates the location (if needed)
			Sets the request.url if location ends with html/htm/php
			*/
			checkUrl(request, url_list);
			if (!request.is_url)
				// Checks redirections, allowed methods and destinations (url)
				checkLocation(request, _location_list);
		}
	}
	catch(const std::exception& e)
	{
		log(e.what(), request.client, "", "", 1);
	}

	if (!kill && request.url != "./favicon.ico")
		log("", request.client, request.server, request.url, 3);
}
