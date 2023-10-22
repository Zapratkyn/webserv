#include "../include/Server.hpp"

using namespace server_utils;

Server::Server() : _host(""), _server_name(""), _root(""), _index(""), _client_max_body_size(-1) { return; }
Server::~Server() { return; }

bool Server::setHost(const std::string &host)
{
	if (_host != "")
		return false;
	_host = host;
	return true;
}
bool Server::setServerName(const std::string &name)
{
	if (_server_name != "")
		return false;
	_server_name = name;
	return true;
}
bool Server::setRoot(std::string &root)
{
	if (_root != "")
		return false;
	if (root.back() != '/')
		root.append("/");
	_root = root;
	return true;
}
bool Server::setBodySize(const std::string &size)
{
	if (_client_max_body_size >= 0 || size.find_first_not_of(DIGITS) != size.npos)
		return false;
	_client_max_body_size = std::stoi(size);
	return true;
}
bool Server::setIndex(const std::string &index)
{
	if (_index != "")
		return false;
	_index = index;
	return true;
}
bool Server::addPort(const std::string &value)
{
	int iValue;

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
	return true;
}
bool Server::addLocation(std::stringstream &ifs, std::string &value)
{
	std::string location_block;

	if (!_location_list.empty())
	{
		if (_location_list.find(value) != _location_list.end())
			return false;
	}
	location_block = getLocationBlock(ifs);
	if (location_block.size())
	{
		_location_list[value] = newLocation(value, location_block);
		if (!_location_list[value].valid)
			return false;
	}
	return true;
}


std::string							Server::getHost() const { return _host; }
std::string 						Server::getServerName() const { return _server_name; }
std::string 						Server::getRoot() const { return _root; }
std::string 						Server::getIndex() const { return _index; }
int 								Server::getBodySize() const { return _client_max_body_size; }
std::vector<int> 					Server::getPorts() const { return _ports; }
std::map<std::string, t_location> 	Server::getLocationlist() const { return _location_list; }

/*
In the 2 functions below :
We use the server_block to parse any option written in it to the server's attributes
If an option doesn't exist or is in double, we throw an error and stop the program
If a line doesn't end with a ';' or a bracket, we throw an error and stop the program
There can be several ports and locations (structures) in a server
There can be several methods in a location
Location names are used to make sure a same location is not used more than once
*/
bool Server::parseOption(const int &option, std::string &value, std::stringstream &ifs, const std::string &server_name)
{
	switch (option) {
		case 0:
			if (!addPort(value))
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

bool Server::parseServer(const std::string &server_block, const std::string &server_name)
{
	std::string 		buffer, name, value, option_list[7] = {"listen", "host", "server_name", "client_max_body_size", "root", "index", "location"};
	std::stringstream	ifs(server_block); // std::stringstream works the same as a std::ifstream but is constructed from a string instead of a file
	int					option;

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
		/*
		Thanks to the 'option == 7' condition, we don't need a default 
		behavior for the switch statement in the parseOption function
		*/
		if (option == 7 || !parseOption(option, value, ifs, server_name))
			return false;
	}
	return true;
}