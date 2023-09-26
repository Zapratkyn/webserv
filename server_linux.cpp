#include "http_tcpServer_linux.hpp"

int main()
{
	TcpServer *server;

	try
	{
		server = new TcpServer("0.0.0.0", 8080);
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