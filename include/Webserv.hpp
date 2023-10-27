#ifndef __WEBSERV_HPP__
# define __WEBSERV_HPP__

#include <ctime>
#include "utils/webserv_utils.hpp"

# define EXIT_SUCCESS 0
# define EXIT_FAILURE 1

# define DISPLAY_URL false
# define DISPLAY_SERVERS false

class Webserv {

private:

	int									_socket;
	fd_set								_readfds;
	std::vector<int>					_socket_list;
	int									_new_socket;
	struct sockaddr_in 					_socketAddr;
	unsigned int						_socketAddrLen;
	// struct timeval					_tv;
	std::map<std::string, std::time_t>	_previous_client;
	std::vector<std::string>			_url_list;

	int									newConnection(int);

	std::string							_conf;
	std::map<std::string, Server*>		_server_list;
	bool								_kill;

public:

	Webserv(const std::string &);
	~Webserv();
	void	startListen();
	void	startServer();
	void	parseConf();

	class openSocketException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't open socket"; } };
	class bindException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't bind socket"; } };
	class listenException : public std::exception { public: virtual const char *what() const throw() { return "ERROR\nCouldn't start listening"; } };
	class confFailureException : public std::exception { public: virtual const char *what() const throw() { return "Configuration failure. Program stopped."; } };

};

#endif
