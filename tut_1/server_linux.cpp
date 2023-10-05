#include "http_tcpServer_linux.hpp"

int main()
{
	TcpServer server(8080);

	try
	{
		server.startServer();
		server.startListen();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() <<std::endl;
		return 1;
	}

	return 0;
}