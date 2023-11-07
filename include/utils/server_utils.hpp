#ifndef __SERVER_UTILS_HPP__
# define __SERVER_UTILS_HPP__

#include <sstream>
#include <vector>
#include <iostream>
#include <map>
#include <unistd.h>
#include "utils.hpp"

# define DISPLAY_METHOD_AND_LOCATION false

# define DIGITS "0123456789"

struct t_location {

	std::string					location;
	std::string					root;
	std::string					index;
	std::vector<std::string>	methods;
	bool						valid;

};

namespace server_utils {

    std::string getOptionName(const std::string &);
    std::string getOptionValue(const std::string &);
    std::string getLocationBlock(std::stringstream &);
    t_location  newLocation(const std::string &, const std::string &);
    void        ft_error(int, std::string, std::string);
	bool		allowedMethod(std::string &, std::vector<std::string> &);
	void		setRequest(t_request &, bool &);
	void		checkUrl(struct t_request &, std::vector<std::string> &);
	void		checkLocation(struct t_request &, std::map<std::string, struct t_location> &, std::vector<std::string> &);
	void		sendTable(struct t_request &, std::vector<std::string> &);
	void		addParentDirectory(std::string &, std::string);
	void		addLink(std::string &, std::string &);

	class invalidMethodException : public std::exception { public: virtual const char *what() const throw() { return "Invalid method"; } };

};

#endif
