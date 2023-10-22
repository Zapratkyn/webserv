#ifndef __SERVER_UTILS_HPP__
# define __SERVER_UTILS_HPP__

#include <iostream>
#include <string>
#include <sstream>
#include <vector>

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

};

#endif