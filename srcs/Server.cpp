#include "../include/Server.hpp"

using namespace server_utils;

Server::Server() : _root(""), _index(""), _autoindex("off"), _client_max_body_size(-1)
{
	return;
}
Server::~Server()
{
	return;
}

bool Server::addServerName(const std::string &name)
{
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
	DIR *dir;

	if (_root != "")
	{
		ft_error(0, root, "root");
		return false;
	}
	dir = opendir(root.c_str());
	if (!dir)
	{
		ft_error(9, root, "root");
		return false;
	}
	if (root[root.size() - 1] != '/')
		root.append("/");
	_root = root;
	closedir(dir);
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
			ft_error(0, value, "listen");
			return false;
		}
	}
	_end_points.push_back(addr_in);
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
		_location_list[value] = newLocation(value, location_block, _root, _autoindex);
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
		default_location.index = "index.html";
	if (_root != "")
		default_location.root = _root;
//	else
//		default_location.root = "www/server00/";
	default_location.location = "/";
	default_location.autoindex = "off";
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
			// TODO check if status_code is not present yet (check of valid url should be done in handling request)
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
bool Server::setAutoIndex(std::string &value)
{
	if (value != "on" && value != "off")
		return false;
	_autoindex = value;
	return true;
}

const std::vector<std::string> &Server::getServerNames() const
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
const std::map<std::string, t_location> &Server::getLocationlist() const
{
	return _location_list;
}
std::vector<struct sockaddr_in> Server::getEndPoints() const
{
	return _end_points;
}
const std::map<int, std::string> &Server::getErrorPages() const
{
	return _error_pages;
}
/*
In the 2 functions below :
We use the server_block to parse any option written in it to the server's attributes
If an option doesn't exist or is in double, we throw an error and stop the program
If a line doesn't start with "location" and doesn't end with a ';' or a bracket, we throw an error and stop the program
There can be several ports and locations (structures) in a server
There can be several methods in a location
Location names are used to make sure a same location is not used more than once
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
	case 7:
		if (!setAutoIndex(value))
			return false;
	}
	return true;
}

bool Server::parseServer(const std::string &server_block)
{
	std::string buffer, name, value,
	    option_list[8] = {"listen", "server_name", "client_max_body_size", "root", "index", "location", "error_page", "autoindex"};
	std::stringstream ifs(server_block); // std::stringstream works the same as a std::ifstream but is constructed from
	                                     // a string instead of a file
	int option;
	t_location folder;
	std::ifstream index;

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
		for (option = 0; option < 8; ++option)
		{
			if (name == option_list[option])
				break;
		}
		/*
		Thanks to the 'option == 8' condition, we don't need a default
		behavior for the switch statement in the parseOption() function
		*/
		if (option == 8)
		{
			ft_error(4, name, "");
			return false;
		}
		if (!parseOption(option, value, ifs))
			return false;
	}
	if (_index == "")
		_index = "index.html";
	if (_client_max_body_size == -1)
		_client_max_body_size = 60000; // The PDF states we need to limit the client_max_body_size
	if (_location_list.find("/") == _location_list.end())
		addDefaultLocation();
	return true;
}
