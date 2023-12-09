#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <iostream>
#include <map>

class Response
{
  public:
	Response();
	virtual ~Response();
	static std::map<int, std::string> all_status_codes;
	int setStatusCode(int status_code);
	int getStatusCode() const;
  private:
	Response(const Response &src);
	Response &operator=(const Response &rhs);
	int _status_code;
};

std::ostream &operator<<(std::ostream &o, const Response &rhs);

#endif
