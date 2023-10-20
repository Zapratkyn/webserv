#include "../include/Server.hpp"

using namespace server_utils;

Server::Server() : _host(""), _server_name(""), _root(""), _index(""), _client_max_body_size(-1) { return; }
Server::~Server() { return; }

void Server::setHost(const std::string &host)
{
	_host = host;
	return;
}
void Server::setServerName(const std::string &name)
{
	_server_name = name;
	return;
}
void Server::setRoot(const std::string &root)
{
	_root = root;
	return;
}
void Server::setBodySize(const int &size)
{
	_client_max_body_size = size;
	return;
}
void Server::setIndex(const std::string &index)
{
	_index = index;
	return;
}


std::string							Server::getHost() const { return _host; }
std::string 						Server::getServerName() const { return _server_name; }
std::string 						Server::getRoot() const { return _root; }
std::string 						Server::getIndex() const { return _index; }
int 								Server::getBodySize() const { return _client_max_body_size; }
std::vector<int> 					Server::getPorts() const { return _ports; }
std::map<std::string, t_location> 	Server::getLocationlist() const { return _location_list; }

/*
In the 3 functions below :
We use the server_block to parse any option written in it to the server's attributes
If an option doesn't exist or is in double, we throw an error and stop the program
If a line doesn't end with a ';' or a bracket, we throw an error and stop the program
There can be several ports and locations (structures) in a server
There can be several methods in a location
Location names are used to make sure a same location is not used more than once
*/
t_location new_location(const std::string &location_name, const std::string &location_block)
{
	t_location loc;
	int option, pos;
	std::stringstream ifs(location_block);
	std::string method, buffer, name, value, option_list[3] = {"root", "index", "allow_methods"};

	loc.location = location_name;
	loc.root = "";
	loc.index = "";
	loc.valid = false;

	while (!ifs.eof())
	{
		getline(ifs, buffer);
		name = getOptionName(buffer);
		value = getOptionValue(buffer);
		for (int i = 0; i <= 3; i++)
		{
			option = i;
			if (name == option_list[i])
				break;
		}
		switch (option) {
			case 0:
				if (loc.root != "")
					return loc;
				loc.root = value;
				break;
			case 1:
				if (loc.index != "")
					return loc;
				loc.index = value;
				break;
			case 2:
				value.push_back(' ');
				while (value[1])
				{
					pos = value.find_first_of(" \t");
					method = value.substr(0, pos);
					if (!loc.methods.empty())
					{
						for (std::vector<std::string>::iterator it = loc.methods.begin(); it != loc.methods.end(); it++)
						{
							if (*it == method)
								return loc;
						}
						loc.methods.push_back(method);
					}
					value = &value[pos];
					value = &value[value.find_first_not_of(" \t")];
				}
				break;
			default:
				return loc;
		}
	}
	loc.valid = true;
	return loc;
}

bool Server::parseOption(const int &option, std::string &value, std::stringstream &ifs, const std::string &server_name)
{
	std::string buffer, location_block = "";
	int iValue;

	switch (option) {
		case 0:
			if (value.find_first_not_of(DIGITS) != value.npos)
				return false;
			iValue = std::stoi(value);
			if (!_ports.empty())
			{
				for (std::vector<int>::iterator it = _ports.begin(); it != _ports.end(); it++)
				{
					if (*it == iValue)
						return false;
				}
			}
			_ports.push_back(iValue);
			break;
		case 1:
			if (_host != "")
				return false;
			_host = value;
			break;
		case 2:
			if (server_name != "")
				return false;
			_server_name = server_name;
			break;
		case 3:
			if (_client_max_body_size >= 0 || value.find_first_not_of(DIGITS) != value.npos)
				return false;
			_client_max_body_size = std::stoi(value);
			break;
		case 4:
			if (_root != "")
				return false;
			_root = value;
			break;
		case 5:
			if (_index != "")
				return false;
			if (value.back() != '/')
				value.append("/");
			_index = value;
			break;
		case 6:
			if (!_location_list.empty())
			{
				if (_location_list.find(value) != _location_list.end())
					return false;
			}
			while (location_block.back() != '}')
			{
				getline(ifs, buffer);
				if (buffer[0] == '{')
					continue;
				location_block.append(buffer);
				if (location_block.back() != '}')
					location_block.append("\n");
			}
			location_block.pop_back();
			if (location_block.back() == '\n')
				location_block.pop_back();
			_location_list[value] = new_location(value, location_block);
			if (!_location_list[value].valid)
				return false;
			break;
	}
	return true;
}

bool Server::parseServer(const std::string &server_block, const std::string &server_name)
{
	std::string 	buffer, name, value, option_list[7] = {"listen", "host", "server_name", "client_max_body_size", "root", "index", "location"};
	std::stringstream	ifs(server_block); // std::stringstream works the same as a std::ifstream but is constructed from a string instead of a file
	int				option;

	while (!ifs.eof())
	{
		getline(ifs, buffer);
		if (!buffer.size()) // Skips empty lines
			continue;
		if (buffer.back() != ';' && buffer.substr(0, buffer.find_first_of(" \t")) != "location")
			return false;
		name = getOptionName(buffer);
		value = getOptionValue(buffer);
		for (int i = 0; i <= 7; i++)
		{
			option = i;
			if (name == option_list[i])
				break;
		}
		if (option == 7 || !parseOption(option, value, ifs, server_name)) // Thanks to the 'option == 7' condition, we don't need a default behavior for the switch statement in the parseOption function
			return false;
	}
	return true;
}