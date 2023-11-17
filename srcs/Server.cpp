#include "../include/Server.hpp"

using namespace server_utils;

Server::Server()
    : _host(""), _root(""), _index(""), _client_max_body_size(-1) {}

Server::~Server() {
  std::vector<int>::const_iterator it;
  for (it = _sockets.begin(); it != _sockets.end(); ++it) {
    close(*it);
  }
}

std::string Server::getHost() const { return _host; }

std::vector<std::string> Server::getServerNames() const {
  return _server_names;
}
std::string Server::getRoot() const { return _root; }

std::string Server::getIndex() const { return _index; }

int Server::getBodySize() const { return _client_max_body_size; }

std::map<std::string, t_location> Server::getLocationlist() const {
  return _location_list;
}

std::vector<int> Server::getSockets() const { return _sockets; }

std::vector<Server::host_port_type> Server::getEndpoints() const {
  return _endpoints;
}

/*
In the 2 functions below :
We use the server_block to parse any option written in it to the server's
attributes If an option doesn't exist or is in double, we throw an error and
stop the program If a line doesn't start with "location" and doesn't end with a
';' or a bracket, we throw an error and stop the program There can be several
ports and locations (structures) in a server There can be several methods in a
location. Location names are used to make sure a same location is not used more
than once
*/
bool Server::parseOption(const int &option, std::string &value,
                         std::stringstream &ifs,
                         const std::string &server_name) {
  switch (option) {
  case 0:
    if (!addEndpoint(value))
      return false;
    break;
  case 1:
    if (!setHost(value))
      return false;
    break;
  case 2:
    if (!addServerName(server_name))
      return false;
    break;
  case 3:
    if (!setBodySize(value))
      return false;
    break;
  case 4:
    if (!setRoot(value))
      return false;
    break;
  case 5:
    if (!setIndex(value))
      return false;
    break;
  case 6:
    if (!addLocation(ifs, value))
      return false;
    break;
  }
  return true;
}

bool Server::parseServer(const std::string &server_block,
                         const std::string &server_name,
                         std::vector<std::string> &folder_list) {
  std::string buffer, name, value,
      option_list[7] = {
          "listen", "host",  "server_name", "client_max_body_size",
          "root",   "index", "location"};
  std::stringstream ifs(
      server_block); // std::stringstream works the same as a std::ifstream but
                     // is constructed from a string instead of a file
  int option;
  t_location folder;

  while (!ifs.eof()) {
    getline(ifs, buffer);
    if (!buffer.size()) // Skips empty lines
      continue;
    name = getOptionName(buffer);
    value = getOptionValue(buffer);
    if (buffer[buffer.size() - 1] != ';') {
      if (buffer.substr(0, buffer.find_first_of(" \t")) != "location") {
        ft_error(1, value, name);
        return false;
      }
    }
    for (int i = 0; i <= 7; i++) {
      option = i;
      if (name == option_list[i])
        break;
    }
    /*
    Thanks to the 'option == 7' condition, we don't need a default
    behavior for the switch statement in the parseOption() function
    */
    if (option == 7) {
      ft_error(4, name, "");
      return false;
    }
    if (!parseOption(option, value, ifs, server_name))
      return false;
  }
  if (_endpoints.empty()) {
    std::cerr << server_name << " needs at least one endpoint" << std::endl;
    return false;
  }
  if (_client_max_body_size == -1)
    _client_max_body_size =
        60000; // The PDF states we need to limit the client_max_body_size
  if (_location_list.find("/") == _location_list.end())
    addDefaultLocation();
  for (std::vector<std::string>::iterator it = folder_list.begin();
       it != folder_list.end(); it++) {
    folder.autoindex = "on";
    folder.root = *it;
    folder.location = *it;
    _location_list[*it] = folder;
  }
  return true;
}

bool Server::initServer() {
  int socket_fd;
  int reuse = true;

  std::vector<host_port_type>::const_iterator it;
  for (it = _endpoints.begin(); it != _endpoints.end(); ++it) {

    struct sockaddr_in addr = {};
    int err = setSocketAddress(it->first, it->second, &addr);
    if (err)
      throw std::runtime_error(std::string(gai_strerror(err)) + " for " +
                               it->first + ":" + it->second);

    if ((socket_fd = socket(addr.sin_family, SOCK_STREAM, 0)) < 0)
      throw std::runtime_error(std::string(strerror(errno)) + " for " +
                               it->first + ":" + it->second);
    _sockets.push_back(socket_fd);

    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
        0)
      throw std::runtime_error(std::string(strerror(errno)) + " for " +
                               it->first + ":" + it->second);

    socklen_t addr_len = sizeof addr;
    if (bind(socket_fd, (struct sockaddr *)&addr, addr_len) < 0)
      throw std::runtime_error(std::string(strerror(errno)) + " for " +
                               it->first + ":" + it->second);

    if (listen(socket_fd, MAX_LISTEN) < 0)
      throw std::runtime_error(std::string(strerror(errno)) + " for " +
                               it->first + ":" + it->second);
  }
  return true;
}

void Server::handleRequest(struct t_request &request,
                           std::vector<std::string> &url_list, bool &kill) {
  try {
    setRequest(request,
               kill); // Gets the method and the location from the request
    if (!kill) {
      /*
      Truncates the location (if needed)
      Sets the request.url if location ends with html/htm/php
      */
      checkUrl(request, url_list);
      if (!request.is_url)
        // Checks redirections, allowed methods and destinations (url)
        checkLocation(request, _location_list);
    }
  } catch (const std::exception &e) {
    log(e.what(), request.client, "", "", 1);
  }

  if (!kill)
    log("", request.client, request.server, request.url, 3);
}
