#ifndef __SERVER_UTILS_HPP__
# define __SERVER_UTILS_HPP__

#include <sstream>
#include <vector>
#include <iostream>
#include <map>
#include <unistd.h>

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
	std::string					url;
	std::string					code;
	std::string					message;
	bool						is_url;

};

namespace server_utils {

    std::string getOptionName(const std::string &);
    std::string getOptionValue(const std::string &);
    std::string getLocationBlock(std::stringstream &);
    t_location  newLocation(const std::string &, const std::string &);
    void        ft_error(int, std::string, std::string);
	bool		validMethod(std::string &);
	bool		allowedMethod(std::string &, std::vector<std::string> &);
	void		killMessage(int);


	// libft
    std::string ft_pop_back(std::string);
    int         ft_stoi(std::string);
	std::string ft_to_string(int);
    std::string trim(const std::string &);

};

#endif
