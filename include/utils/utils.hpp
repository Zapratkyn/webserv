#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <arpa/inet.h>
#include <csignal>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <fstream>
#include <iostream>
#include <map>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sstream>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <vector>

//#include <arpa/inet.h>
//#include <dirent.h>
#include <netdb.h>


#define DISPLAY_HTML false
#define DISPLAY_METHOD_AND_LOCATION false
#define DISPLAY_REQUEST false
#define DISPLAY_URL false
#define DISPLAY_SERVERS false

#define DIGITS "0123456789"
#define BUFFER_SIZE 10000
#define MAX_LISTEN 1000

struct t_request {

  std::string method;
  std::string location;
  std::string url;
  std::string code;
  std::string server;
  std::string client;
  std::string header;
  std::string body;
  bool is_url;
  int socket;
};

std::string ft_pop_back(std::string);
int ft_stoi(std::string);
std::string ft_to_string(int);
std::string trim(const std::string &);
void log(std::string, std::string, std::string, std::string, int);
void		sendText(struct t_request &);
void		sendFile(struct t_request &);
std::string getContentType(std::string);

#endif