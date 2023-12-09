#include "../../include/messages/Request.hpp"

Request::Request(int socket)
    : _socket(socket), _method(), _request_target(), _http_version(), _headers(), _body(), _error_status(0),
      _error_message(), _chunked_request(false)
{
}

Request::Request(const Request &src)
    : _socket(src._socket), _method(src._method), _request_target(src._request_target),
      _http_version(src._http_version), _headers(src._headers), _body(src._body), _error_status(src._error_status),
      _error_message(src._error_message), _chunked_request(src._chunked_request)
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
	_error_message = rhs._error_message;
	_chunked_request = rhs._chunked_request;
	return *this;
}

Request::~Request()
{
}

bool Request::operator==(const Request &rhs) const
{
	return this->_socket == rhs._socket;
}

void Request::_parseRequestLine(const std::string &line)
{
	std::stringstream ss(line);

	ss >> _method >> _request_target >> _http_version;

	std::string tmp;
	if (ss >> tmp)
	{
		_error_status = 400;
		_error_message = "Invalid request-line";
	}
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

void Request::_getMessageBody(std::stringstream &ss)
{
	// TODO first check if what is set in relation to the message body()
	if (!isChunkedRequest())
		_body.insert(_body.end(), ss.str().begin(), ss.str().end());
	else
		; // parse function for chunked body; and see if all is there
}

void Request::_parseRequest(const char *buffer)
{
	std::stringstream ss(buffer);
	std::string line;

	getline(ss, line);
	line.erase(std::remove(line.begin(), line.end(), '\r' ), line.end());
	_parseRequestLine(line);

	while (!_error_status)
	{
		getline(ss, line);
		line.erase(std::remove(line.begin(), line.end(), '\r' ), line.end());
		if (line.find(':') != std::string::npos)
			_parseHeader(line);
		else if (line.empty())
		{
			if (_headers.empty())
			{
				_error_status = 400;
				_error_message = "Invalid empty line after request-line";
			}
			else if (ss.rdbuf()->in_avail() != 0)
				_getMessageBody(ss);
			return;
		}
		else
		{
			_error_status = 400;
			_error_message = "Invalid header";
		}
	}
}

bool Request::getRequest()
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
		std::cout << "****** Request Message on socket " << _socket << " (Received) ******" << std::endl;
		std::cout << buffer << "[EOF]" << std::endl;
		std::cout << "******* Request Message on socket " << _socket << " (Parsed) *******" << std::endl;
		std::cout << *this << std::endl;
	}

	if (!_error_status && !_chunked_request)
		; // validate request part1 (cf. rfc shizzle)

	if (DISPLAY_REQUEST)
	{
		std::cout << "***** Request Message on socket " << _socket << " (Valid pt1) ******" << std::endl;
		std::cout << *this << std::endl;
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

const std::string &Request::getErrorMessage() const
{
	return _error_message;
}

std::ostream &operator<<(std::ostream &o, const Request &rhs)
{
	o << "Error status: " << rhs.getErrorStatus() << std::endl;
	if (rhs.getErrorStatus())
		o << "Error message: " << rhs.getErrorMessage() << std::endl;
	o << "Method: " << rhs.getMethod() << std::endl;
	o << "Request Target: " << rhs.getRequestTarget() << std::endl;
	o << "HTTP Version: " << rhs.getHTTPVersion() << std::endl;
	o << "Headers: " << std::endl;
	std::map<std::string, std::vector<std::string> >::const_iterator it = rhs.getHeaders().begin();
	for (; it != rhs.getHeaders().end(); ++it)
	{
		o << "- " << it->first << std::endl;
		std::vector<std::string>::const_iterator it2 = it->second.begin();
		for (; it2 != it->second.end(); ++it2)
			o << "   - " << *it2 << std::endl;
	}
	o << "Body: " << std::endl;
	o << rhs.getBody() << "[EOL]" << std::endl;
	o << "*****************************************" << std::endl;
	return o;
}
