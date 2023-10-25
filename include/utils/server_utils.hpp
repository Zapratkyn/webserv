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

// struct t_request {

// 	std::string					method;

// };

namespace server_utils {

    std::string getOptionName(const std::string &);
    std::string getOptionValue(const std::string &);
    std::string getLocationBlock(std::stringstream &);
    t_location  newLocation(const std::string &, const std::string &);
    void        ft_error(int, std::string, std::string);
    std::string ft_pop_back(std::string);
    int         ft_stoi(std::string);

};

#endif
