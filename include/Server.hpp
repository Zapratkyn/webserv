#ifndef __SERVER_HPP__
# define __SERVER_HPP__

#include <map>
#include <unistd.h>
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
	std::string									_request_header;
	std::string									_request_body;

	bool 										parseOption(const int &, std::string &, std::stringstream &, const std::string &, std::vector<int> &);
	bool										getRequest(int);

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

	bool										parseServer(const std::string &, const std::string &, std::vector<int> &);
	void										handle_request(int);

	class readRequestException : public std::exception { public: virtual const char *what() const throw() { return "Error while reading request"; } };
	class requestBodyTooBigException : public std::exception { public: virtual const char *what() const throw() { return "Error while reading request"; } };


};

#endif
