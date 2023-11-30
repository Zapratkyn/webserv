#include "../include/ErrorPage.hpp"

static std::map<int, std::string> getStatuses()
{
	std::map<int, std::string> m;
	m[400] = "Bad Request";
	m[401] = "Unauthorized";
	m[403] = "Forbidden";
	m[404] = "Not Found";
	m[405] = "Method Not Allowed";
	return m;
}

std::map<int, std::string> ErrorPage::_statuses = getStatuses();

ErrorPage::ErrorPage(int status_code, Server *server) : _status_code(status_code), _server(server)
{
	std::map<int, std::string> map = _server->getErrorPages();
	std::map<int, std::string>::const_iterator it;
	it = map.find(_status_code);
	if (it == map.end())
		_buildErrorPage();
	else
		_retrieveErrorPage();
}

ErrorPage::~ErrorPage()
{
}

void ErrorPage::_buildErrorPage()
{
	std::string meaning = _statuses[_status_code];

	_stream << "<!DOCTYPE html>\n"
	           "<head>\n"
	           "\t<meta charset=\"UTF-8\">\n"
	           "\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
	           "\t<title>" + meaning + "</title>\n"
	           "</head>\n\n"
	           "<body>\n"
	           "\t\t\t<h1>" << _status_code << "</h1>\n"
	           "\t\t\t<h3>" + meaning + "</h3>\n"
	           "</body>\n"
	           "</html>";
}

void ErrorPage::_retrieveErrorPage()
{
	std::string url = _server->getErrorPages().find(_status_code)->second;
	// location should be prepended to get the full path
	//  stat or access to check the file is available as file
	std::ifstream ifs(url);
	std::string s;
	if (ifs.is_open())
	{
		_stream << ifs.rdbuf();
		ifs.close();
	}
	else
	{
		// throw an error for log
		_buildErrorPage();
	}
}

std::string ErrorPage::getErrorPageAsString() const
{
	return (_stream.str());
}

const std::stringstream &ErrorPage::getErrorPageAsStream() const
{
	return _stream;
}


std::ostream &operator<<(std::ostream &o, const ErrorPage &rhs)
{
	(void)rhs;
	return o;
}
