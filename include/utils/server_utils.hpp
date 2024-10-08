#ifndef __SERVER_UTILS_HPP__
#define __SERVER_UTILS_HPP__

#include "utils.hpp"

struct t_request;

struct t_location
{

	std::string location;
	std::string root;
	std::string index;
	std::vector<std::string> methods;
	std::string autoindex;
	bool valid;
};

namespace server_utils
{

std::string getOptionName(const std::string &);
std::string getOptionValue(const std::string &);
std::string getLocationBlock(std::stringstream &);
t_location newLocation(const std::string &, const std::string &, std::string &, std::string);
void parseFolders(std::string &);
void ft_error(int, std::string, std::string);
bool allowedMethod(std::string &, std::vector<std::string> &);
void setResponse(t_request &, bool &, std::string);
void checkUrl(struct t_request &, std::string, std::string &);
bool checkLocation(struct t_request &, std::map<std::string, struct t_location> &, std::string);
// bool checkRedirection(struct t_request &);
bool displayRoot(struct t_request &, std::string, std::string);
void sendTable(struct t_request &, std::string, std::string);
void addParentDirectory(std::string &, std::string, std::string);
void addLinkList(std::string &, std::string, std::string);
bool setSocketAddress(const std::string &ip_address, const std::string &port_num, struct sockaddr_in *socket_addr);
int getSocketAddress(int socket, struct sockaddr_in *addr);


class invalidMethodException : public std::exception
{
  public:
	virtual const char *what() const throw()
	{
		return "Invalid method";
	}
};

}; // namespace server_utils

#endif
