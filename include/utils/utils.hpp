#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include "../Server.hpp"
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

#define DISPLAY_HTML false
#define DISPLAY_METHOD_AND_LOCATION false
#define DISPLAY_REQUEST true
#define DISPLAY_URL false
#define DISPLAY_SERVERS true

#define DIGITS "0123456789"
#define BUFFER_SIZE 10000
#define MAX_LISTEN 1000

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define ERROR 0
#define PAGE 1
#define FILE 2
#define DIRECTORY 3

class Server;

struct t_request
{

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
	int socket;
	int type;
};

std::string ft_pop_back(std::string);
int ft_stoi(std::string);
std::string ft_to_string(int);
std::string trim(const std::string &);
void log(std::string, std::string, std::string, int);
bool sendText(struct t_request &);
void sendFile(struct t_request &);
void sendError(int, int);
std::string getContentType(std::string);

#endif