#include "../../include/messages/Request.hpp"

Request::Request(int socket)
    : _socket(socket), _error_status(0), _chunked_request(false), _content_length(0), _server(nullptr),
      _response(nullptr)
{
}

Request::Request(int socket, Server *server)
    : _socket(socket), _error_status(0), _chunked_request(false), _content_length(0), _server(server),
      _response(nullptr)
{
}

Request::Request(const Request &src)
    : _socket(src._socket), _method(src._method), _request_target(src._request_target),
      _http_version(src._http_version), _headers(src._headers), _body(src._body), _error_status(src._error_status),
      _chunked_request(src._chunked_request), _content_length(src._content_length), _server(src._server),
      _server_location(src._server_location), _response(src._response)
{
}

Request &Request::operator=(const Request &rhs)
{
	if (this == &rhs)
		return *this;
	_socket = rhs._socket;
	_method = rhs._method;
	_http_version = rhs._http_version;
	_headers = rhs._headers;
	_body = rhs._body;
	_error_status = rhs._error_status;
	_chunked_request = rhs._chunked_request;
	_content_length = rhs._content_length;
	_server = rhs._server;
	_server_location = rhs._server_location;
	_response = rhs._response;
	return *this;
}

Request::~Request()
{
	delete _response;
}

bool Request::operator==(const Request &rhs) const
{
	return this->_socket == rhs._socket;
}

void Request::_parseRequestLine(const std::string &line)
{
	std::stringstream ss(line);
	ss >> _method >> _request_target >> _http_version;
}

void Request::_parseHeader(const std::string &line)
{
	size_t pos = line.find(':');
	std::string key = line.substr(0, pos);
	std::string tmp = line.substr(pos + 1);
	std::vector<std::string> value;

	std::istringstream iss(tmp);
	while (getline(iss, tmp, ','))
	{
		tmp = trim(tmp);
		value.push_back(tmp);
	}
	_headers.insert(std::make_pair(key, value));
}

void Request::_retrieveBodyInfo()
{
	if (_headers.count("Content-Length") == 1)
	{
		if (_chunked_request)
			_error_status = 400;
		std::stringstream ss;
		ss << _headers["Content-Length"][0];
		ss >> _content_length;
	}
	if (_headers.count("Transfer-Encoding") == 1)
	{
		_chunked_request = true;
	}
}

void Request::_parseBody(std::stringstream &ss)
{
	std::stringstream buf;
	buf << ss.rdbuf();
	if (!isChunkedRequest())
		_body = buf.str();
	else
		; // TODO parse function for chunked body
}

void Request::_parseRequest(const char *buffer)
{
	std::stringstream ss(buffer);
	std::string line;

	getline(ss, line);
	line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
	_parseRequestLine(line);

	while (!_error_status)
	{
		getline(ss, line);
		line.erase(std::remove(line.begin(), line.end(), '\r'), line.end());
		if (line.find(':') != std::string::npos)
			_parseHeader(line);
		else if (line.empty())
		{
			_validateParsedRequestLine(line);
			_validateParsedHeaders();
			if (!_error_status)
			{
				_retrieveBodyInfo();
				if (_error_status)
					return;
				_parseBody(ss);
				_validateParsedBody();
			}
			return;
		}
		else
			_error_status = 400;
	}
}

void Request::_validateParsedRequestLine(const std::string &line)
{
	(void)line;
	// TODO check if there are still things in the line after _http_version
	if (_method.empty() || _request_target.empty() || _http_version.empty())
		_error_status = 400;
	else if (!webserv_utils::methodIsImplemented(_method))
		_error_status = 501;
	else if (!webserv_utils::httpVersionIsSupported(_http_version))
		_error_status = 505;
}

void Request::_validateParsedHeaders()
{
	if (_error_status)
		return;
	if (_headers.empty() || _headers.count("Host") == 0 ||
	    (_headers.count("Content-Length") == 1 && _headers.count("Transfer-Encoding") == 1))
		_error_status = 400;
}

void Request::_validateParsedBody()
{
	if (_body.size() != _content_length)
	{
		if (_body.size() > _content_length)
			_error_status = 413;
		else
			_error_status = 400;
	}
	// TODO what whith chunked request?
	else if (!_body.empty() && _headers.count("Content-Length") == 0)
		_error_status = 411;
}


void Request::_resetRequest()
{
	Server *server = this->_server;
	int socket = this->_socket;
	delete this->_response;  // TODO can we do this in the overloaded assignment operator???
	*this = Request(socket, server);
}

bool Request::retrieveRequest()
{
	ssize_t bytes;
	char buffer[BUFFER_SIZE] = {};

	bytes = recv(_socket, buffer, BUFFER_SIZE, 0);
	if (bytes < 0)
		throw Request::readRequestException();
	else if (bytes == 0)
	{
		log("closed connection", _socket, "", 1);
		return false;
	}

	_parseRequest(buffer);

	if (DISPLAY_REQUEST)
	{
		std::cout << "****** Request on socket " << _socket << " (Received) ******" << std::endl;
		std::cout << buffer << "[EOF]" << std::endl;
		std::cout << "******* Request on socket " << _socket << " (Parsed) *******" << std::endl;
		std::cout << *this;
	}
	return true;
}

int Request::getSocket() const
{
	return _socket;
}

const std::string &Request::getMethod() const
{
	return _method;
}

const std::string &Request::getRequestTarget() const
{
	return _request_target;
}

const std::string &Request::getHTTPVersion() const
{
	return _http_version;
}

const std::map<std::string, std::vector<std::string> > &Request::getHeaders() const
{
	return _headers;
}

const std::string &Request::getBody() const
{
	return _body;
}

bool Request::isChunkedRequest() const
{
	return _chunked_request;
}
int Request::getErrorStatus() const
{
	return _error_status;
}

size_t Request::getContentLength() const
{
	return _content_length;
}

bool Request::getValueOfHeader(const std::string &key, std::vector<std::string> &value) const
{
	std::map<std::string, std::vector<std::string> >::const_iterator it;
	it = _headers.find(key);
	if (it == _headers.end())
		return false;
	value = it->second;
	return true;
}

std::ostream &operator<<(std::ostream &o, const Request &rhs)
{
	o << "[ INFO ]" << std::endl;
	o << "_error_status: " << rhs.getErrorStatus() << std::endl;
	o << "_body.size(): " << rhs.getBody().size() << std::endl;
	o << "_content_length: " << rhs.getContentLength() << std::endl;
	o << "_chunked_request: " << std::boolalpha << rhs.isChunkedRequest() << std::endl;
	o << "[ PARSED ]" << std::endl;
	o << "_method: " << rhs.getMethod() << std::endl;
	o << "_request_target: " << rhs.getRequestTarget() << std::endl;
	o << "_http_version: " << rhs.getHTTPVersion() << std::endl;
	o << "_headers: " << std::endl;
	std::map<std::string, std::vector<std::string> >::const_iterator it = rhs.getHeaders().begin();
	for (; it != rhs.getHeaders().end(); ++it)
	{
		o << "   " << it->first << std::endl;
		std::vector<std::string>::const_iterator it2 = it->second.begin();
		for (; it2 != it->second.end(); ++it2)
			o << "      " << *it2 << std::endl;
	}
	o << "_body: " << std::endl;
	o << rhs.getBody() << "[EOL]" << std::endl;
	o << "*****************************************" << std::endl << std::endl;
	return o;
}
