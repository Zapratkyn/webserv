#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include "utils/server_utils.hpp"
#include "utils/utils.hpp"
// #include "ErrorPage.hpp"
#include <map>
#include <vector>
#include <string>

struct t_location;
// class ErrorPage;

class Server
{

  private:
	std::vector<std::string> _server_name;
	std::string _root;
	std::string _index;
	std::string _autoindex;
	int _client_max_body_size;
	std::map<std::string, t_location> _location_list;
	std::vector<struct sockaddr_in> _end_points;
	std::map<int, std::string> _error_pages;
	// ErrorPage	_errorPage;

	bool parseOption(const int &, std::string &, std::stringstream &);
	void addDefaultLocation();

  public:
	Server();
	~Server();

	std::vector<std::string> getServerNames() const;
	std::string getRoot() const;
	std::string getIndex() const;
	int getBodySize() const;
	std::map<std::string, t_location> getLocationlist() const;
	std::vector<struct sockaddr_in> getEndPoints() const;
	std::map<int, std::string> getErrorPages() const;

	bool addServerName(const std::string &);
	bool setRoot(std::string &);
	bool setIndex(const std::string &);
	bool setBodySize(const std::string &);
	bool addEndPoint(const std::string &);
	bool addLocation(std::stringstream &, std::string &);
	bool addErrorPage(std::string &);
	bool setAutoIndex(std::string &);
	bool parseServer(const std::string &);
	void handleRequest(struct t_request &, bool &);
};

#endif
