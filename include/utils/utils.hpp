#ifndef __UTILS_HPP__
# define __UTILS_HPP__

#include <iostream>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <algorithm>
#include <vector>
#include <map>
#include <dirent.h>
#include <ctime>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>
#include <netdb.h>
#include "../Server.hpp"

# define DISPLAY_HTML false
# define DISPLAY_METHOD_AND_LOCATION false
# define DISPLAY_REQUEST false
# define DISPLAY_URL false
# define DISPLAY_SERVERS false

# define DIGITS "0123456789"
# define BUFFER_SIZE 10000
# define MAX_LISTEN 1000

# define EXIT_SUCCESS 0
# define EXIT_FAILURE 1

class Server;

struct t_request {

	std::string					method;
	std::string					location;
	std::string					url;
	std::string					code;
	std::vector<Server*>		potentialServers;
    Server                 		*server;
    std::string                 client;
    std::string                 header;
    std::string                 body;
	std::string					host;
	bool						is_url;
	int							socket;

};

std::string ft_pop_back(std::string);
int         ft_stoi(std::string);
std::string ft_to_string(int);
std::string trim(const std::string &);
void        log(std::string, std::string, std::string, int);
bool		sendText(struct t_request &);
void		sendFile(struct t_request &);
void		sendError(int, int);
std::string getContentType(std::string);

#endif