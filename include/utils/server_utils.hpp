#ifndef __SERVER_UTILS_HPP__
# define __SERVER_UTILS_HPP__

#include <sstream>
#include <vector>
#include <iostream>
#include <map>
#include <unistd.h>
#include "utils.hpp"

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

};

#endif
