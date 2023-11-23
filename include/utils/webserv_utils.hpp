#ifndef __WEBSERV_UTILS_HPP__
#define __WEBSERV_UTILS_HPP__

#include "../Server.hpp"
#include "utils.hpp"

namespace webserv_utils {

bool checkRedirectionList(std::vector<std::string> &);
std::string getServerName(const std::string &, int &,
                          std::map<std::string, Server *> &);
std::string getServerBlock(std::ifstream &);
void initSockaddr(struct sockaddr_in &);
void ft_error(int, std::string);
void listenLog(struct sockaddr_in &, std::map<std::string, Server *> &);
std::string getServer(std::map<std::string, Server *> &, int &);
void displayServers(std::map<std::string, Server *> &);
void parseUrl(std::string, std::vector<std::string> &,
              std::vector<std::string> &);
void readRequests(std::map<std::string, Server *> &, std::map<int, t_request> &,
                  fd_set &);
void getRequest(int, struct t_request &);
bool validMethod(std::string &);
void initRequest(struct t_request &);

class readRequestException : public std::exception {
public:
  virtual const char *what() const throw() {
    return "Error while reading request";
  }
};
class requestBodyTooBigException : public std::exception {
public:
  virtual const char *what() const throw() {
    return "Client's request's body too big";
  }
};
class forbiddenMethodException : public std::exception {
public:
  virtual const char *what() const throw() { return "Forbidden method"; }
};

}; // namespace webserv_utils

#endif
