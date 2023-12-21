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
      _chunked_response(false), _dir_listing(false)
{
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

void Response::buildMessage()
{
	if (_status_code >= 400)
		_buildErrorBody();
	_buildStatusLine();
	_buildHeaders();

	_message = _status_line + "\r\n" + _headersAsString + "\r\n" + _body;
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

	std::map<std::string, std::vector<std::string> >::const_iterator cit;
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

bool Response::_retrieveResponseBody(const std::string &path)
{
	if (_dir_listing)
		return (_buildDirListing());
	std::ifstream ifs(path);
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

static std::vector<std::string> getDirEntries(const std::string &dir_path)
{
	std::vector<std::string> result;
	DIR *dir;
	struct dirent *entry;

	dir = opendir(dir_path.c_str());
	if (dir)
	{
		while ((entry = readdir(dir)))
		{
			if (entry->d_type == DT_DIR)
				result.push_back(std::string(entry->d_name) + "/");
			else
				result.push_back(entry->d_name);
		}
		closedir(dir);
	}
	return result;
}

bool Response::_buildDirListing()
{
	std::stringstream ss;

	std::vector<std::string> dir_entries = getDirEntries(_resource_path);

	ss << "<!DOCTYPE html>"
	      "<html>\n"
	      "<head><title>"
	   << "Index of " + UrlParser(_request->_request_target).path +
	          "</title></head>\n"
	          "<body>\n"
	   << "<h1>" "Index of " + UrlParser(_request->_request_target).path + "</h1><hr><pre>";

	std::vector<std::string>::const_iterator cit;
	for (cit = dir_entries.begin(); cit != dir_entries.end(); ++cit)
		ss << "<a href=\"" + *cit + "\">" + *cit + "</a>\n";

	ss << "</pre><hr></body>\n"
	      "</html>";

	_body = ss.str();

	return true;
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

	if (!isValidDirectory(root) || access(root.c_str(), R_OK | X_OK) < 0)
		return false;

	_resource_path = root;
	std::string error_page = _request->_server->getErrorPages().find(_status_code)->second;
	_resource_path += error_page;

	if (!isValidFile(_resource_path) || access(root.c_str(), R_OK) < 0)
	{
		_resource_path.clear();
		return false;
	}

	if (!_retrieveResponseBody(_resource_path))
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

void Response::sendMessage()
{
	if (_message.size() > BUFFER_SIZE)
		; // TODO do chunked
	ssize_t bytes_sent = send(_request->_socket, _message.c_str(), _message.size(), 0);
	(void)bytes_sent;
}

std::ostream &operator<<(std::ostream &o, const Response &rhs)
{
	o << "[ MESSAGE INFO ]" << std::endl;
	o << "Path to resource : " << rhs._resource_path << std::endl;
	o << "[ MESSAGE ]" << std::endl;
	o << rhs._message << "[EOL]";
	return o;
}

void Response::handleRequest()
{
	if (_request->_method == "GET")
		_doGet();
	else if (_request->_method == "POST")
		_doPost();
	else if (_request->_method == "DELETE")
		_doDelete();
}

void Response::_doGet()
{
	if (!_request->_body.empty() ||
	    (_request->_headers.count("Content-Length") && _request->_headers["Content-Length"][0] != "0"))
	{
		_status_code = 400;
		return;
	}
	_setResourcePath();

	t_location location = _request->_server->getLocationlist().find(_request->_server_location)->second;

	if (isValidDirectory(_resource_path) && location.autoindex == "on")
	{
		if (isValidFile(_resource_path + location.index))
		{
			if (access((_resource_path + location.index).c_str(), R_OK) == 0)
				_resource_path += location.index;
			else
			{
				_resource_path.clear();
				_status_code = 403;
				return;
			}
		}
		else if (access(_resource_path.c_str(), R_OK | X_OK) < 0)
		{
			_resource_path.clear();
			_status_code = 403;
			return;
		}
		else
			_dir_listing = true;
	}
	else if (isValidFile(_resource_path))
	{
		if (access(_resource_path.c_str(), R_OK) < 0)
		{
			_resource_path.clear();
			_status_code = 403;
			return;
		}
	}
	else
	{
		_status_code = 404;
		_resource_path.clear();
		return;
	}

	if (_retrieveResponseBody(_resource_path))
		_status_code = 200;
}


//TODO parsing of _body in case Content-Type ==
// - multipart/form-data
// - application/x-www-form-urlencoded
//TODO https://developer.mozilla.org/en-US/docs/Learn/Forms/Sending_and_retrieving_form_data
void Response::_doPost()
{

}

void Response::_doDelete()
{
	_setResourcePath();

	if (isValidDirectory(_resource_path))
	{
		if (access(_resource_path.c_str(), W_OK | X_OK) < 0)
		{
			_resource_path.clear();
			_status_code = 403;
		}
		else
		{
			//TODO how to delete a directory ????
			_status_code = 204;
		}
	}
	else if (isValidFile(_resource_path))
	{
		if (access(_resource_path.c_str(), W_OK) < 0)
		{
			_resource_path.clear();
			_status_code = 403;
		}
		else
		{
			std::remove(_resource_path.c_str());
			_status_code = 204;
		}
	}
	else
	{
		_status_code = 404;
		_resource_path.clear();
	}
}
