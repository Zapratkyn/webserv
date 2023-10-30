#ifndef __UTILS_HPP__
# define __UTILS_HPP__

#include <string>

struct t_request {

	std::string					method;
	std::string					location;
	std::string					url;
	std::string					code;
    std::string                 server;
    std::string                 client;
	bool						is_url;
    bool						is_kill;

};

std::string ft_pop_back(std::string);
int         ft_stoi(std::string);
std::string ft_to_string(int);
std::string trim(const std::string &);

#endif