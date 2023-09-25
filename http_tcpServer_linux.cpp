#include "http_tcpServer_linux.hpp"

TcpServer::TcpServer(std::string ip_address, int port) : _ip_address(ip_address), _port(port)
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	return;
}
TcpServer::~TcpServer() 
{
	if (_socket >= 0)
		close(_socket);
	return;
}

int TcpServer::getSocket() const { return _socket; }