#include "../include/Server.hpp"

using namespace server_utils;

Server::Server() : _host(""), _server_name(""), _root(""), _index(""), _client_max_body_size(0) { return; }
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


std::string	Server::getHost() const { return _host; }
std::string Server::getServerName() const { return _server_name; }
std::string Server::getRoot() const { return _root; }
std::string Server::getIndex() const { return _index; }
int Server::getBodySize() const { return _client_max_body_size; }
std::vector<int> Server::getPorts() const { return _ports; }
std::map<std::string, t_location> Server::getLocationlist() const { return _location_list; }

t_location new_location(std::string &location_name, std::string &location_block)
{
	t_location loc;
	int option, pos;
	std::ifstream ifs(location_block);
	std::string method, buffer, name, value, option_list[3] = {"root", "index", "allow_methods"};

	loc.location = location_name;
	loc.root = "";
	loc.index == "";
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
				}
				break;
			default:
				return loc;
		}
	}
	loc.valid = true;
	return loc;
}

bool Server::parseOption(int option, std::string &value, std::ifstream &ifs)
{
	std::string buffer = "", tmp;
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
			if (_server_name != "")
				return false;
			_server_name = value;
			break;
		case 3:
			if (_client_max_body_size)
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
			_index = value;
			break;
		case 6:
			if (!_location_list.empty())
			{
				if (_location_list.find(value) != _location_list.end())
					return false;
			}
			while (tmp.back() != '}')
			{
				getline(ifs, buffer);
				if (buffer[0] == '{')
					continue;
				tmp.append(buffer);
				if (tmp.back() != '}')
					tmp.append("\n");
			}
			tmp.pop_back();
			tmp.pop_back();
			_location_list[value] = new_location(value, tmp);
			if (!_location_list[value].valid)
				return false;
			break;
	}
	return true;
}

bool Server::parseServer(std::string server_block)
{
	std::string 	buffer, name, value, option_list[7] = {"listen", "host", "server_name", "client_max_body_size", "root", "index", "location"};
	std::ifstream	ifs(server_block);
	int				pos, option;

	while (!ifs.eof())
	{
		getline(ifs, buffer);
		if (!buffer.size())
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
		if (option == 8 || !parseOption(option, value, ifs))
			return false;
	}
	return true;
}