#include "../../include/messages/Response.hpp"

static std::map<int, std::string> getStatusCodes()
{
	std::map<int, std::string> m;
	m[200] = "OK";
	m[400] = "Bad Request";
	m[401] = "Unauthorized";
	m[403] = "Forbidden";
	m[404] = "Not Found";
	m[405] = "Method Not Allowed";
	m[501] = "Not implemented";
	m[413] = "Content Too Large";
	return m;
}

std::map<int, std::string> Response::all_status_codes = getStatusCodes();


Response::Response()
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

std::ostream &operator<<(std::ostream &o, const Response &rhs)
{
	(void)rhs;
	// o << ;
	return o;
}
