#ifndef __SERVER_UTILS_HPP__
#define __SERVER_UTILS_HPP__

#include "parse_utils.hpp"
#include "utils.hpp"
#include <arpa/inet.h>
#include <dirent.h>
#include <iostream>
#include <map>
#include <netdb.h>
#include <sstream>
#include <unistd.h>
#include <vector>

#define DISPLAY_METHOD_AND_LOCATION true

namespace server_utils {

bool allowedMethod(std::string &, std::vector<std::string> &);
void setRequest(t_request &, bool &);
void checkUrl(struct t_request &, std::vector<std::string> &);
void checkLocation(struct t_request &,
                   std::map<std::string, struct t_location> &);
void sendTable(struct t_request &, std::string);
void addParentDirectory(std::string &, std::string);
void addLinkList(std::string &, std::string);
int setSocketAddress(const std::string &ip_address, const std::string &port_num,
                     struct sockaddr_in *socket_addr);
int getSocketAddress(int socket, struct sockaddr_in *addr);
void printSocketAddress(struct sockaddr_in &_socketAddr);

class invalidMethodException : public std::exception {
public:
  virtual const char *what() const throw() { return "Invalid method"; }
};

}; // namespace server_utils

#endif
