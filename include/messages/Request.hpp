#ifndef REQUEST_HPP
#define REQUEST_HPP

#include <iostream>
#include <map>
#include <vector>
#include "../utils/utils.hpp"
#include <utility>

class Request
{
  public:
	explicit Request(int socket);
	Request(const Request &src);
	Request &operator=(const Request &rhs);
	virtual ~Request();
	bool operator==(const Request &rhs) const;



	bool getRequest();



	int getSocket() const;
	const std::string &getMethod() const;
	const std::string &getRequestTarget() const;
	const std::string &getHTTPVersion() const;
	const std::map<std::string, std::vector<std::string> >&getHeaders() const;
	const std::string &getBody() const;
	bool isChunkedRequest() const;
	int getErrorStatus() const;
	const std::string &getErrorMessage() const;
	class readRequestException : public std::exception
	{
	  public:
		virtual const char *what() const throw()
		{
			return "Error while reading request";
		}
	};

  private:
	Request();
	int _socket;
	std::string _method;
	std::string _request_target;
	std::string _http_version;
	std::map<std::string, std::vector<std::string > > _headers;
	std::string _body;
	int _error_status;
	std::string _error_message;
	bool _chunked_request;

	void _parseRequest(const char *buffer);
	void _parseRequestLine(const std::string &line);
	void _parseHeader(const std::string &line);
	void _getMessageBody(std::stringstream &ss);
};

std::ostream &operator<<(std::ostream &o, const Request &rhs);

#endif
