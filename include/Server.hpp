#ifndef __SERVER_HPP__
# define __SERVER_HPP__

#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <dirent.h>
#include <fstream>
#include "utils/server_utils.hpp"


#define DIGITS "0123456789"

class Server {

private:

	std::string									_host;
	std::string									_server_name;
	std::string									_root;
	std::string									_index;
	int											_client_max_body_size;
	std::vector<int>							_ports;
	std::vector<int>							_sockets;
	std::map<std::string, t_location>			_location_list;
	std::vector<std::string>					_url_list;

	bool 										parseOption(const int &, std::string &, std::stringstream &, const std::string &, std::vector<int> &);
	void										addDefaultLocation();
	void										getRequest(int, std::string &, std::string &);
	void										setRequest(t_request &, std::string &, std::string &, bool &);
	void										sendUrl(t_request &, int);
	std::string									buildResponse();

public:

	Server();
	~Server();

	std::string									getHost() const;
	std::string 								getServerName() const;
	std::string 								getRoot() const;
	std::string 								getIndex() const;
	int											getBodySize() const;
	std::vector<int> 							getPorts() const;
	std::map<std::string, t_location> 			getLocationlist() const;
	std::vector<int>							getSockets() const;

	bool										setHost(const std::string&);
	bool										setServerName(const std::string&);
	bool										setRoot(std::string&);
	bool										setIndex(const std::string&);
	bool										setBodySize(const std::string&);
	bool										addPort(const std::string&, std::vector<int>&);
	bool										addLocation(std::stringstream&, std::string &value);
	void										addSocket(int&);

	bool										parseServer(const std::string &, const std::string &, std::vector<int> &, std::vector<std::string> &);
	void										handleRequest(int, struct sockaddr_in &, bool &);

	class readRequestException : public std::exception { public: virtual const char *what() const throw() { return "Error while reading request"; } };
	class requestBodyTooBigException : public std::exception { public: virtual const char *what() const throw() { return "Client's request' body too big"; } };
	class invalidMethodException : public std::exception { public: virtual const char *what() const throw() { return "Invalid method"; } };
	class forbiddenMethodException : public std::exception { public: virtual const char *what() const throw() { return "Forbidden method"; } };


};

#endif
