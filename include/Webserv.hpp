#ifndef __WEBSERV_HPP__
#define __WEBSERV_HPP__

#include "utils/utils.hpp"
#include "utils/webserv_utils.hpp"
#include "utils/UrlParser.hpp"
#include "messages/Request.hpp"
#include "messages/Response.hpp"
#include <algorithm>
#include <vector>


// For Linux
#ifndef FD_COPY
# define FD_COPY(from, to) bcopy(from, to, sizeof(*(from)));
#endif

class Request;

class Webserv
{

  private:
	std::vector<int> _listen_socket_list;
	std::vector<int> _global_socket_list;
	std::vector<Request *> _request_list;
	std::vector<Server *> _server_list;

	bool acceptNewConnections(fd_set &);
	void readRequests(fd_set &, fd_set &);
	void sendResponses(fd_set &, fd_set &);
	std::string _conf;
	std::map<int, struct sockaddr_in> _socket_list;
  public:
	static std::vector<std::string> implementedMethods;
	Webserv();
	explicit Webserv(const std::string &);
	~Webserv();
	void startListen();
	void startServer();
	void parseConf();

	class openSocketException : public std::exception
	{
	  public:
		virtual const char *what() const throw()
		{
			return "ERROR\nCouldn't open socket";
		}
	};
	class bindException : public std::exception
	{
	  public:
		virtual const char *what() const throw()
		{
			return "ERROR\nCouldn't bind socket";
		}
	};
	class listenException : public std::exception
	{
	  public:
		virtual const char *what() const throw()
		{
			return "ERROR\nCouldn't start listening";
		}
	};
	class confFailureException : public std::exception
	{
	  public:
		virtual const char *what() const throw()
		{
			return "Configuration failure. Program stopped.";
		}
	};
	class logError : public std::exception
	{
	  public:
		virtual const char *what() const throw()
		{
			return "LOG ERROR.";
		}
	};
	class redirectionListException : public std::exception
	{
	  public:
		virtual const char *what() const throw()
		{
			return "Redirection list failure. Program stopped.";
		}
	};
	class setSocketoptionException : public std::exception
	{
	  public:
		virtual const char *what() const throw()
		{
			return "ERROR\nSet socket option failed";
		}
	};
};

#endif