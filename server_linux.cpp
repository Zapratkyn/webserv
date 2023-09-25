#include "http_tcpServer_linux.hpp"

int main()
{
	TcpServer server("0.0.0.0", 8080);

	if (server.getSocket() < 0)
	{
		std::cerr << "ERROR:\nCouldn't create socket" << std::endl;
		return 1;
	}
	

	return 0;
}