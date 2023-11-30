#ifndef ERRORPAGE_HPP
#define ERRORPAGE_HPP

#include <iostream>
#include <map>
#include "Server.hpp"

class ErrorPage
{
  public:
	ErrorPage(int status_code, Server *server);
	virtual ~ErrorPage();
	std::string getErrorPageAsString() const;
	const std::stringstream &getErrorPageAsStream() const;

  private:
	ErrorPage(const ErrorPage &src);
	ErrorPage();
	ErrorPage &operator=(const ErrorPage &rhs);
	static std::map<int, std::string> _statuses;
	const int _status_code;
	const Server *_server;
	void _buildErrorPage();
	void _retrieveErrorPage();
	std::stringstream _stream;
};

std::ostream &operator<<(std::ostream &o, const ErrorPage &rhs); //make this a friend???

#endif
