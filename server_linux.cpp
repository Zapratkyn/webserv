#include "http_tcpServer_linux.hpp"

int main()
{
	TcpServer *server; // A pointer because exceptions are possible in the constructor

	try
	{
		server = new TcpServer("0.0.0.0", 8080);
		server->startListen();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() <<std::endl;
		delete server;
		return 1;
	}
	
	delete server;

	return 0;
}