#include "../include/Server.hpp"

using namespace server_utils;

Server::Server() : _root(""), _index(""), _client_max_body_size(-1)
{
	return;
}
Server::~Server()
{
	return;
}

bool Server::addServerName(const std::string &name)
{
	if (!_server_name.empty())
	{
		ft_error(0, name, "server_name");
		return false;
	}
	std::istringstream ss(name);
	std::string token;
	while (ss >> token)
	{
		if (std::find(_server_name.begin(), _server_name.end(), name) != _server_name.end())
		{
			ft_error(0, token, "server_name");
			return false;
		}
		else
			_server_name.push_back(token);
	}
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
bool Server::addEndPoint(const std::string &value)
{
	std::string ip_address;
	std::string port_num;
	struct sockaddr_in addr_in;

	if (value.empty())
	{
		ft_error(6, "\"\"", "listen");
		return false;
	}
	size_t pos = value.find(':');
	if (pos != std::string::npos)
	{
		ip_address = value.substr(0, pos++);
		if (ip_address.empty())
		{
			ft_error(7, value, "listen");
			return false;
		}
		if (ip_address == "*")
			ip_address = "0.0.0.0";
		port_num = value.substr(pos);
	}
	else if (value.find('.') != std::string::npos || value.find_first_not_of(DIGITS) != std::string::npos)
	{
		ip_address = value;
		port_num = "8080";
	}
	else if (value.find_first_not_of(DIGITS) == std::string::npos)
	{
		port_num = value;
		ip_address = "0.0.0.0";
	}

	if (!setSocketAddress(ip_address, port_num, &addr_in))
		return false;

	for (std::vector<struct sockaddr_in>::iterator it = _end_points.begin(); it != _end_points.end(); ++it)
	{
		if (it->sin_addr.s_addr == addr_in.sin_addr.s_addr && it->sin_port == addr_in.sin_port)
		{
			ft_error(2, value, "error_page");
			return false;
		}
	}
	_end_points.push_back(addr_in);
	ip_address.append(":");
	ip_address.append(port_num);
	_hosts.push_back(ip_address);
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

bool Server::addErrorPage(std::string &value)
{
	std::istringstream ss(value);
	std::string token;
	std::vector<std::string> tmp;
	int code;
	bool code_is_set = false;
	std::string url;

	while (ss >> token)
		tmp.push_back(token);

	while (!tmp.empty())
	{
		if (!code_is_set)
		{
			if (tmp[0].find_first_not_of(DIGITS) != std::string::npos)
			{
				ft_error(2, tmp[0], "error_page");
				return false;
			}
			ss.str("");
			ss.clear();
			ss.str(tmp[0]);
			ss >> code;
			code_is_set = true;
		}
		else
		{
			// TODO check valid url and code not present yet
			_error_pages.insert(std::make_pair(code, tmp[0]));
			code_is_set = false;
		}
		if (tmp.size() == 1 && code_is_set)
		{
			ft_error(8, tmp[0], "error_page");
			return false;
		}
		tmp.erase(tmp.begin());
	}
	return true;
}

std::vector<std::string> Server::getHosts() const
{
	return _hosts;
}
std::vector<std::string> Server::getServerNames() const
{
	return _server_name;
}
std::string Server::getRoot() const
{
	return _root;
}
std::string Server::getIndex() const
{
	return _index;
}
int Server::getBodySize() const
{
	return _client_max_body_size;
}
std::map<std::string, t_location> Server::getLocationlist() const
{
	return _location_list;
}
std::vector<struct sockaddr_in> Server::getEndPoints() const
{
	return _end_points;
}
std::map<int, std::string> Server::getErrorPages() const
{
	return _error_pages;
}

/*
In the 2 functions below :
We use the server_block to parse any option written in it to the server's
attributes If an option doesn't exist or is in double, we throw an error and
stop the program If a line doesn't start with "location" and doesn't end with a
';' or a bracket, we throw an error and stop the program There can be several
ports and locations (structures) in a server There can be several methods in a
location Location names are used to make sure a same location is not used more
than once
*/
bool Server::parseOption(const int &option, std::string &value, std::stringstream &ifs)
{
	switch (option)
	{
	case 0:
		if (!addEndPoint(value))
			return false;
		break;
	case 1:
		if (!addServerName(value))
			return false;
		break;
	case 2:
		if (!setBodySize(value))
			return false;
		break;
	case 3:
		if (!setRoot(value))
			return false;
		break;
	case 4:
		if (!setIndex(value))
			return false;
		break;
	case 5:
		if (!addLocation(ifs, value))
			return false;
		break;
	case 6:
		if (!addErrorPage(value))
			return false;
		break;
	}
	return true;
}

bool Server::parseServer(const std::string &server_block, std::vector<std::string> &folder_list)
{
	std::string buffer, name, value,
	    option_list[7] = {"listen", "server_name", "client_max_body_size", "root", "index", "location", "error_page"};
	std::stringstream ifs(server_block); // std::stringstream works the same as a std::ifstream but
	                                     // is constructed from a string instead of a file
	int option;
	t_location folder;

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
		for (option = 0; option < 7; option++)
		{
			if (name == option_list[option])
				break;
		}
		if (option == 7)
		{
			ft_error(4, name, "");
			return false;
		}
		if (!parseOption(option, value, ifs))
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
	std::string extension;
	try
	{
		setRequest(request,
		           kill); // Gets the method and the location from the request
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
	catch (const std::exception &e)
	{
		log(e.what(), request.client, "", 1);
	}

	extension = request.url.substr(request.url.find_last_of("."));

	// if (!kill && (request.url == "./dir.html" || request.url.substr(0, 6) ==
	// "./www/"))
	if (!kill && extension != ".css" && extension != ".ico")
		log("", request.client, request.url, 3);
}
