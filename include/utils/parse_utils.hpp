#ifndef __PARSE_UTILS_HPP__
#define __PARSE_UTILS_HPP__

#include <arpa/inet.h>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <string>
#include <utility>
#include <vector>
#include <sstream>
#include "utils.hpp"

#define DIGITS "0123456789"


struct t_location {

  std::string location;
  std::string root;
  std::string index;
  std::vector<std::string> methods;
  std::string autoindex;
  bool valid;
};

void ft_error(int, std::string, std::string);
bool validFile(const std::string &);
std::string getOptionName(const std::string &);
std::string getOptionValue(const std::string &);
std::string getLocationBlock(std::stringstream &);
t_location newLocation(const std::string &, const std::string &);


#endif