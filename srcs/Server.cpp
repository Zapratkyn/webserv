#include "../include/Server.hpp"

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