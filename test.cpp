#include <netinet/in.h>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <arpa/inet.h>

int main()
{
	struct addrinfo hints;
	struct addrinfo *res;
	struct sockaddr_in *addr;

	hints.ai_family = AF_INET;
  	hints.ai_socktype = SOCK_STREAM;
  	hints.ai_flags = AI_NUMERICSERV;

	getaddrinfo("127.0.0.2", "8080", &hints, &res);

	*addr = *(struct sockaddr_in *)res->ai_addr;
	freeaddrinfo(res);

	std::cout << addr->sin_addr.s_addr << "\n" << addr->sin_port << std::endl;
	return 0;
}