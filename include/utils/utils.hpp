#ifndef __UTILS_HPP__
# define __UTILS_HPP__

#include <algorithm>
#include <arpa/inet.h>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <signal.h>
#include <sstream>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "../Server.hpp"

#define DISPLAY_SERVERS false
#define DISPLAY_REQUEST false
#define DISPLAY_SERVER_FOR_REQUEST false
#define DISPLAY_LOCATION_FOR_REQUEST false
#define DISPLAY_RESPONSE false

#define DIGITS "0123456789"
#define BUFFER_SIZE 10000
#define MAX_LISTEN 1000 //TODO check if os can set this

class Server;

struct t_request
{

	std::map<std::string, std::vector<std::string> > headers;
	std::string method;
	std::string location;
	std::string url;
	std::string code;
	std::vector<Server *> potentialServers;
	Server *server;
	std::string client;
	std::string header;
	std::string body;
	std::string host;
	std::string local;
	int socket;
	bool is_chunked;
};

std::string ft_pop_back(std::string);
int ft_stoi(std::string);
std::string ft_to_string(int);
std::string trim(const std::string &);
void log(std::string, int, std::string, int);
bool sendText(struct t_request &);
void sendFile(struct t_request &);
void sendError(int, int);
std::string getContentType(std::string);
std::string getLocalFolder(std::string);

#endif