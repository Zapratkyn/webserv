#include "http_tcpServer_linux.hpp"

TcpServer::TcpServer(std::string ip_address, int port) : _ip_address(ip_address), _port(port)
{
	_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (_socket < 0)
		throw openSocketException();
	init_addr();
	if (bind(_socket, (sockaddr *)&_addr, sizeof(_addr)) < 0)
		throw bindException();
	if (listen(_socket, 10) < 0)
		throw listenException();
	listen_log();
	return;
}
TcpServer::~TcpServer() 
{
	if (_socket >= 0)
		close(_socket);
	return;
}

void TcpServer::init_addr()
{
	_addr.sin_family = AF_INET;
	_addr.sin_port = htons(_port);
	_addr.sin_addr.s_addr = inet_addr(_ip_address.c_str());
}
void TcpServer::listen_log() const
{
	std::ostringstream ss;
    ss << "\n*** Listening on ADDRESS: " 
        << inet_ntoa(_addr.sin_addr) 
        << " PORT: " << ntohs(_addr.sin_port) 
        << " ***\n\n";
	std::cout << ss.str() << std::endl;
}