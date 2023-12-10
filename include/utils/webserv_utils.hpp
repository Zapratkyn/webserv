#ifndef __WEBSERV_UTILS_HPP__
#define __WEBSERV_UTILS_HPP__

#include "utils.hpp"
#include "../Server.hpp"

namespace webserv_utils
{

bool checkRedirectionList(std::vector<std::string> &);
std::string getServerBlock(std::ifstream &);
void ft_error(int, std::string);
void displayServers(std::vector<Server *> &);
void parseUrl(std::string, std::vector<std::string> &, std::vector<std::string> &);
bool methodIsImplemented(const std::string &method);
bool httpVersionIsSupported(const std::string &version);
bool socketIsSet(std::map<int, struct sockaddr_in> &, struct sockaddr_in &);
void freeSockets(std::vector<int> &, std::vector<int> &, fd_set &, std::vector<struct t_request> &);
void deleteRequest(int, std::vector<struct t_request> &);
void printSocketAddress(struct sockaddr_in &_socketAddr);

class readRequestException : public std::exception
{
  public:
	virtual const char *what() const throw()
	{
		return "Error while reading request";
	}
};
class requestBodyTooBigException : public std::exception
{
  public:
	virtual const char *what() const throw()
	{
		return "Client's request's body too big";
	}
};
class forbiddenMethodException : public std::exception
{
  public:
	virtual const char *what() const throw()
	{
		return "Forbidden method";
	}
};

}; // namespace webserv_utils

#endif
