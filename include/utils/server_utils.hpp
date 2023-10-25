#ifndef __SERVER_UTILS_HPP__
# define __SERVER_UTILS_HPP__

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <map>

struct t_location {

	std::string					location;
	std::string					root;
	std::string					index;
	std::vector<std::string>	methods;
	bool						valid;

};

struct t_request {

	std::string					method;
	std::string					location;

};

namespace server_utils {

    std::string getOptionName(const std::string &);
    std::string getOptionValue(const std::string &);
    std::string getLocationBlock(std::stringstream &);
    t_location  newLocation(const std::string &, const std::string &);
    void        ft_error(int, std::string, std::string);
    std::string ft_pop_back(std::string);
    int         ft_stoi(std::string);
	bool		validMethod(std::string &);
	bool		allowedMethod(std::string &, std::vector<std::string> &);

};

#endif
