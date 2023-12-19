#include "../../include/messages/Response.hpp"

static std::map<int, std::string> getStatusCodes()
{
	std::map<int, std::string> m;
	m[200] = "OK";
	m[201] = "Created";
	m[202] = "Accepted";
	m[204] = "No Content";
	m[400] = "Bad Request";
	m[401] = "Unauthorized";
	m[403] = "Forbidden";
	m[404] = "Not Found";
	m[405] = "Method Not Allowed";
	m[413] = "Content Too Large";
	m[411] = "Length Required";
	m[501] = "Not Implemented";
	m[500] = "Internal Server Error";
	m[507] = "Insufficient Storage";
	m[505] = "HTTP Version Not Supported";
	return m;
}

std::map<int, std::string> Response::_all_status_codes = getStatusCodes();

Response::Response(Request *request)
    : _request(request), _status_code(request->_error_status), _http_version("HTTP/1.1"), _content_length(0),
      _chunked_response(false)
{
}

Response::Response(const Response &src)
{
	(void)src;
}

Response &Response::operator=(const Response &rhs)
{
	if (this == &rhs)
		return *this;
	return *this;
}

Response::~Response()
{
}

static bool isValidDirectory(const std::string &path)
{
	struct stat sb = {};
	return (stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode));
}

static bool isValidFile(const std::string &path)
{
	struct stat sb = {};
	return (stat(path.c_str(), &sb) == 0 && S_ISREG(sb.st_mode));
}

static bool checkPermissions(const std::string &path, bool read, bool write, bool execute)
{
	if (read && access(path.c_str(), R_OK) < 0)
		return false;
	if (write && access(path.c_str(), W_OK) < 0)
		return false;
	if (execute && access(path.c_str(), X_OK) < 0)
		return false;
	return true;
}

void Response::handleCgi()
{
	char **argv = NULL;
	char **env = NULL;

	argv[0] = strdup(_request->getBody().c_str());
	env[0] = strdup(ft_to_string(_request->_server->getBodySize()).c_str());

	_status_code = execve(&_resource_path[1], argv, env);

	if (_status_code != 200)
	{
		_resource_path.clear();
		_buildErrorBody();
	}
	free(argv[0]);
	free(env[0]);
}

void Response::buildMessage()
{
	std::string extension, message;

	if (_status_code != 0)
		_buildErrorBody();
	else
	{
		_setResourcePath();
		extension = &_resource_path[_resource_path.rfind('.')];
		if (extension != ".cgi" && (!isValidFile(_resource_path) || !checkPermissions(_resource_path, true, false, false)))
		{
			_status_code = 404; //TODO diff between 403 and 404???
			_resource_path.clear();
			_buildErrorBody();
		}
		else if (extension == ".cgi")
			handleCgi();
		else if (!_retrieveMessageBody(_resource_path))
			_buildErrorBody();
		else
			_status_code = 200;
	}
	_buildStatusLine();
	_buildHeaders();

	_message = _status_line + "\r\n" + _headersAsString + "\r\n";
	if (DISPLAY_RESPONSE)
	{
		std::cout << "*************** RESPONSE ****************" << std::endl;
		std::cout << *this << std::endl;
		std::cout << "*****************************************" << std::endl << std::endl;
	}
}

void Response::_buildStatusLine()
{
	std::stringstream ss;
	ss << _status_code;

	_status_line = _http_version + " " + ss.str() + " " + _all_status_codes[_status_code];
}

void Response::_buildHeaders()
{
	std::stringstream ss;
	std::string tmp;

	ss << _body.size();
	_headers["Content-Length"].push_back(ss.str());

	if (_request->_headers.count("Connection"))
		tmp = _request->_headers.find("Connection")->second[0];
	if (tmp.empty())
		tmp = "keep-alive";
	_headers["Connection"].push_back(tmp);

	tmp = UrlParser(_resource_path).file_extension;
	_headers["Content-Type"].push_back(tmp);


	std::map<std::string, std::vector<std::string > >::const_iterator cit;
	std::vector<std::string>::const_iterator cite;
	for (cit = _headers.begin(); cit != _headers.end(); ++cit)
	{
		_headersAsString += cit->first + ": ";
		for (cite = cit->second.begin(); cite != cit->second.end(); ++cite)
		{
			_headersAsString += *cite;
			if (cite != cit->second.end() - 1)
				_headersAsString += ',';
		}
		_headersAsString += "\r\n";
	}
}

bool Response::_retrieveMessageBody(const std::string &path)
{
	std::ifstream ifs(path, std::ifstream::binary);
	if (ifs.is_open())
	{
		std::stringstream ss;
		ss << ifs.rdbuf();
		_body = ss.str();
		ifs.close();
		return true;
	}
	else
	{
		if (_status_code == 0)
			_status_code = 500;
		return false;
	}
}

void Response::_buildDefaultErrorBody()
{
	std::stringstream ss;
	ss << "<!DOCTYPE html>"
	      "<html>\n"
	      "<head><title>"
	      << _status_code
	      << " " + _all_status_codes[_status_code] +
	             "</title></head>\n"
	             "<body>\n"
	             "<center><h1>"
	      << _status_code
	      << " " + _all_status_codes[_status_code] +
	             "</h1></center>\n"
	             "</body>\n"
	             "</html>";
	_body = ss.str();
}

void Response::_setResourcePath()
{
	std::string requested_path = UrlParser(_request->_request_target).path;
	std::string tmp = requested_path.substr(_request->_server_location.size());
	std::string root = _request->_server->getLocationlist().find(_request->_server_location)->second.root;
	_resource_path = root + tmp;
}

bool Response::_buildCustomErrorBody()
{
	if (!_request->_server->getErrorPages().count(_status_code))
		return false;

	std::string root = _request->_server->getRoot();

	//TODO is this check needed?
	if (!isValidDirectory(root) || !checkPermissions(root, true, false, true))
		return false;

	_resource_path = root;
	std::string error_page = _request->_server->getErrorPages().find(_status_code)->second;
	_resource_path += error_page;

	if (!isValidFile(_resource_path) || !checkPermissions(_resource_path, true, false, false))
	{
		_resource_path.clear();
		return false;
	}

	if (!_retrieveMessageBody(_resource_path))
	{
		_resource_path.clear();
		return false;
	}

	return true;
}

void Response::_buildErrorBody()
{
	if (!_buildCustomErrorBody())
		_buildDefaultErrorBody();
}

const std::string &Response::getResourcePath() const
{
	return _resource_path;
}

void Response::_chunkReponse()
{

	size_t pos1 = _message.find("Content-Length"), pos2 = _message.find("\r\nContent-Type"), copied = 0;
	std::ifstream file(_resource_path, std::ifstream::binary);
	std::stringstream converter;
	std::vector<char> buffer;
	char c;
	
	_message.replace(pos1, pos2 - pos1, "Transfer-Encoding: chunked");
	_body = "";

	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		if (file.eof())
			break;
		copied++;
		file.read(&c, 1);
		buffer.push_back(c);
	}

	while (copied > 0)
	{
		converter.str("");
		converter << std::hex << copied;
		_body.append(converter.str());
		_body.append("\r\n");
		for(std::vector<char>::iterator it = buffer.begin(); it != buffer.end(); it++)
			_body.append(1, *it);
		_body.append("\r\n");
		buffer.clear();
		copied = 0;
		for (int i = 0; i < BUFFER_SIZE; i++)
		{
			if (file.eof())
				break;
			copied++;
			file.read(&c, 1);
			buffer.push_back(c);
		}
	}

	file.close();
	_body.append("0\r\n\r\n");
}

void Response::sendMessage()
{
	if (_body.size() > BUFFER_SIZE)
		_chunkReponse(); //TODO do chunked
	_message.append(_body);
	ssize_t bytes_sent = send(_request->_socket, _message.c_str(), _message.size(), 0);
	if (bytes_sent < 0)
		throw Response::sendResponseException();
}

std::ostream &operator<<(std::ostream &o, const Response &rhs)
{
	o << "[ MESSAGE INFO ]" << std::endl;
	o << "Path to resource : " << rhs._resource_path << std::endl;
	o << "[ HEADER ]" << std::endl;
	o << rhs._message << "[EOL]";
	return o;
}
