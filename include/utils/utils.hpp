#ifndef __UTILS_HPP__
# define __UTILS_HPP__

#include <iostream>
#include <ctime>
#include <fstream>
#include <unistd.h>

# define DISPLAY_HTML false

struct t_request {

	std::string					method;
	std::string					location;
	std::string					url;
	std::string					code;
    std::string                 server;
    std::string                 client;
    std::string                 header;
    std::string                 body;
	bool						is_url;
	int							socket;

};

std::string ft_pop_back(std::string);
int         ft_stoi(std::string);
std::string ft_to_string(int);
std::string trim(const std::string &);
void        log(std::string, std::string, std::string, std::string, int);
void		sendUrl(struct t_request &);
void		sendFile(struct t_request &);

#endif