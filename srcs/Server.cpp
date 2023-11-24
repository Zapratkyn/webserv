#include "../include/Server.hpp"

using namespace server_utils;

int Server::ID = 0;

Server::Server()
    : _serverID(ID++), _root(""), _index(""), _client_max_body_size(-1) {}

Server::~Server() {
  std::vector<int>::const_iterator it;
  for (it = _sockets.begin(); it != _sockets.end(); ++it) {
    close(*it);
  }
}

//TODO check if server_name is valid
bool Server::addServerName(const std::string &name) {
  std::istringstream ss(name);
  std::string token;
  while (ss >> token) {
    if (std::find(_server_names.begin(), _server_names.end(), name) !=
        _server_names.end()) {
      ft_error(0, token, "server_name");
      return false;
    } else {
      _server_names.push_back(token);
    }
  }
  return true;
}

bool Server::setRoot(std::string &root) {
  std::string slash = "/";

  if (_root != "") {
    ft_error(0, root, "root");
    return false;
  }
  // We need root path to start and end with '/'
  if (root[0] != '/')
    root = slash.append(root);
  if (root[root.size() - 1] != '/')
    root.append("/");
  if (root == "/")
    root = "/www/";
  _root = root;
  return true;
}

bool Server::setBodySize(const std::string &size) {
  if (_client_max_body_size >= 0 ||
      size.find_first_not_of(DIGITS) != size.npos) {
    ft_error(2, size, "client_max_body_size");
    return false;
  }
  _client_max_body_size = ft_stoi(size);
  return true;
}

bool Server::setIndex(const std::string &index) {
  if (_index != "") {
    ft_error(0, index, "index");
    return false;
  }
  _index = index;
  return true;
}

bool Server::addEndpoint(const std::string &value) {
  std::string ip_address;
  std::string port_num;

  if (value.empty()) {
    ft_error(6, "\"\"", "listen");
    return false;
  }
  size_t pos = value.find(':');
  if (pos != std::string::npos) {
    ip_address = value.substr(0, pos);
    if (ip_address.empty()) {
      ft_error(5, value, "listen");
      return false;
    }
    if (ip_address == "*")
      ip_address = "0.0.0.0";
    port_num = value.substr(pos + 1);
  } else if (value.find('.') != std::string::npos ||
             value.find_first_not_of(DIGITS) != std::string::npos) {
    ip_address = value;
    port_num = "80";
  } else if (value.find_first_not_of(DIGITS) == std::string::npos) {
    port_num = value;
    ip_address = "0.0.0.0";
  }

  host_port_type endpoint = std::make_pair(ip_address, port_num);
  std::vector<host_port_type>::const_iterator it;
  for (it = _endpoints.begin(); it != _endpoints.end(); ++it) {
    if (it->first == endpoint.first && it->second == endpoint.second) {
      ft_error(0, value, "listen");
      return false;
    }
  }
  _endpoints.push_back(endpoint);
  return true;
}

bool Server::addLocation(std::stringstream &ifs, std::string &value) {
  std::string location_block, slash = "/";

  if (!_location_list.empty()) {
    if (_location_list.find(value) != _location_list.end()) {
      ft_error(0, value, "location");
      return false;
    }
  }
  location_block = getLocationBlock(ifs);
  if (location_block.size()) {
    value = slash.append(value);
    _location_list[value] = newLocation(value, location_block);
    if (!_location_list[value].valid)
      return false;
  }
  return true;
}

void Server::addSocket(int socket) { _sockets.push_back(socket); }

void Server::addDefaultLocation() {
  t_location default_location;

  if (_index != "")
    default_location.index = _index;
  else
    default_location.index = "pages/index.html";
  if (_root != "")
    default_location.root = _root;
  else
    default_location.root = "/www/";
  default_location.location = "/";
  default_location.autoindex = "on";
  default_location.methods.push_back("GET");

  _location_list["/"] = default_location;
}

int Server::getServerID() const { return _serverID; }
std::vector<std::string> Server::getServerNames() const { return _server_names; }
std::string Server::getRoot() const { return _root; }

std::string Server::getIndex() const { return _index; }

int Server::getBodySize() const { return _client_max_body_size; }
std::vector<int> Server::getPorts() const { return _ports; }
std::map<std::string, t_location> Server::getLocationlist() const {
  return _location_list;
}

std::vector<int> Server::getSockets() const { return _sockets; }

std::vector<struct sockaddr_in> Server::getSocketAddresses() const {
  return _sock_addrs;
}

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
<<<<<<< HEAD
location. Location names are used to make sure a same location is not used more
than once
*/
bool Server::parseOption(const int &option, std::string &value,
                         std::stringstream &ifs) {
  switch (option) {
  case 0:
    if (!addEndpoint(value))
      return false;
    break;
  case 1:
    if (!addServerName(value))
      return false;
    break;
  case 2:
    if (!setBodySize(value))
      return false;
    break;
  case 3:
    if (!setRoot(value))
      return false;
    break;
  case 4:
    if (!setIndex(value))
      return false;
    break;
  case 5:
    if (!addLocation(ifs, value))
      return false;
    break;
  }
  return true;
}

bool Server::parseServer(const std::string &server_block,
                         std::vector<std::string> &folder_list) {
  std::string buffer, name, value,
      option_list[6] = {
          "listen", "server_name", "client_max_body_size",
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
    if (!parseOption(option, value, ifs))
      return false;
  }
  if (_endpoints.empty()) {
    std::cerr << "server " << _serverID << " needs at least one endpoint" << std::endl;
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

static bool addrIsEqual(const struct sockaddr_in &addr1,
                        const struct sockaddr_in &addr2) {
  return (addr1.sin_addr.s_addr == addr2.sin_addr.s_addr &&
          addr1.sin_port == addr2.sin_port &&
          addr1.sin_family == addr2.sin_family);
}

static bool addrIsUsed(const std::vector<struct sockaddr_in> &addrs,
                       const struct sockaddr_in &addr) {
  std::vector<struct sockaddr_in>::const_iterator it = addrs.begin();
  for (; it != addrs.end(); ++it) {
    if (addrIsEqual(*it, addr))
      return true;
  }
  return false;
}

// TODO is it ok to have a new fd if the addr is already in use by another
//  server or is it better to reuse the fd the addr is already bound to?
bool Server::initServer(std::vector<struct sockaddr_in> &webserv_sock_addrs,
                        std::vector<int> &webserv_sockets) {
  int socket_fd;
  int reuse = true;

  std::vector<host_port_type>::const_iterator it;
  for (it = _endpoints.begin(); it != _endpoints.end(); ++it) {
    struct sockaddr_in addr = {};
    // Get the address based on ip_address and port.
    int err = setSocketAddress(it->first, it->second, &addr);
    if (err)
      throw std::runtime_error(std::string(gai_strerror(err)) + " for " +
                               it->first + ":" + it->second);
    // Check if the address is a duplicate in the server block.
    // A server block can not have 2 listen entries with the same host:port.
    if (addrIsUsed(_sock_addrs, addr))
      throw std::runtime_error("Duplicate of " + it->first + ":" + it->second);
    // Get a socket fd for the address.
    if ((socket_fd = socket(addr.sin_family, SOCK_STREAM, 0)) < 0)
      throw std::runtime_error(std::string(strerror(errno)) + " for " +
                               it->first + ":" + it->second);
    // This should prevent the error can't bind socket when quitting and
    // restarting webserv.
    if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) <
        0)
      throw std::runtime_error(std::string(strerror(errno)) + " for " +
                               it->first + ":" + it->second);
    // Set the sockets to non-blocking
    fcntl(socket_fd, F_SETFL, O_NONBLOCK);
    // Only bind the socket to the address if it wasn't already bound by another
    // server block. 2 server blocks can listen to the same host:port
    if (!addrIsUsed(webserv_sock_addrs, addr)) {
      socklen_t addr_len = sizeof addr;
      if (bind(socket_fd, (struct sockaddr *)&addr, addr_len) < 0)
        throw std::runtime_error(std::string(strerror(errno)) + " for " +
                                 it->first + ":" + it->second);
    }
    // Add the fds and addresses to the server and to the webserv instance
    _sockets.push_back(socket_fd);
    webserv_sockets.push_back(socket_fd);
    _sock_addrs.push_back(addr);
    webserv_sock_addrs.push_back(addr);
    // Listen to incoming connections
    if (listen(socket_fd, MAX_LISTEN) < 0)
      throw std::runtime_error(std::string(strerror(errno)) + " for " +
                               it->first + ":" + it->second);
  }
  return true;
}

void Server::handleRequest(struct t_request &request,
                           std::vector<std::string> &url_list, bool &kill) {
  std::string extension;
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

  extension = request.url.substr(request.url.find_last_of("."));

  // if (!kill && (request.url == "./dir.html" || request.url.substr(0, 6) ==
  // "./www/"))
  if (!kill && extension != ".css" && extension != ".ico")
    log("", request.client, request.server, request.url, 3);
}
