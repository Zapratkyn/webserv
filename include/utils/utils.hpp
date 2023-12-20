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
#include <csignal>
#include <sstream>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>
#include "../Server.hpp"

#define DISPLAY_SERVERS true
#define DISPLAY_REQUEST true
#define DISPLAY_SERVER_FOR_REQUEST true
#define DISPLAY_LOCATION_FOR_REQUEST true
#define DISPLAY_RESPONSE true

#define DIGITS "0123456789"
#define BUFFER_SIZE 10000
#define MAX_LISTEN 1000 //TODO check if os can set this

std::string ft_pop_back(const std::string &);
int ft_stoi(const std::string &);
std::string trim(const std::string &);
void log(const std::string &line, int client_fd, const std::string &url, int type);

#endif