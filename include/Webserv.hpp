#ifndef __WEBSERV_HPP__
# define __WEBSERV_HPP__

#include <fcntl.h>
#include "utils/webserv_utils.hpp"
#include "utils/utils.hpp"

# define EXIT_SUCCESS 0
# define EXIT_FAILURE 1

# define DISPLAY_URL false
# define DISPLAY_SERVERS false

# define MAX_LISTEN 1000

class Webserv {

private:

	std::vector<int>					_listen_socket_list;
	std::map<int, struct t_request>		_request_list;
	struct sockaddr_in 					_socketAddr;
	unsigned int						_socketAddrLen;
	struct timeval						_tv;
	std::vector<std::string>			_url_list;
	std::vector<std::string>			_folder_list;

	void								acceptNewConnections(int &, fd_set &);
	void								readRequests(fd_set &, fd_set &);
	void								sendRequests(bool &, fd_set &);

	std::string							_conf;
	std::map<std::string, Server*>		_server_list;

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
	class logError : public std::exception { public: virtual const char *what() const throw() { return "LOG ERROR."; } };

};

#endif