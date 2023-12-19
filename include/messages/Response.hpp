#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <map>
#include "Request.hpp"
#include <sys/stat.h>
#include "../utils/UrlParser.hpp"

class Request;

class Response
{
  public:
	Response(Request *request);
	virtual ~Response();
	void handleRequest();
	void buildMessage();
	void sendMessage();
	const std::string &getResourcePath() const;
	friend std::ostream &operator<<(std::ostream &o, const Response &rhs);

  private:
	Response();
	Response(const Response &src);
	Response &operator=(const Response &rhs);
	static std::map<int, std::string> _all_status_codes;
	Request *_request;
	int _status_code;
	std::string _http_version;
	std::string _status_line;
	std::map<std::string, std::vector<std::string > > _headers;
	size_t _content_length;
	std::string _headersAsString;
	std::string _body;
	std::string _message;

	bool _chunked_response;

	std::string _resource_path;
	void _setResourcePath();

	void _buildHeaders();
	void _buildStatusLine();
	bool _retrieveMessageBody(const std::string &path);

	void _buildErrorBody();
	void _buildDefaultErrorBody();
	bool _buildCustomErrorBody();

	void _doGETmethod();
	void _doPOSTmethod();
	void _doDELETEmethod();
};

#endif
