#include "../include/Server.hpp"

bool Server::setHost(const std::string &host) {
  if (_host != "") {
    ft_error(0, host, "host");
    return false;
  }
  _host = host;
  return true;
}

bool Server::addServerName(const std::string &name) {
  if (std::find(_server_names.begin(), _server_names.end(), name) ==
      _server_names.end()) {
    _server_names.push_back(name);
    return true;
  } else {
    ft_error(0, name, "server_name");
    return false;
  }
}

bool Server::setRoot(std::string &root) {
  std::string slash = "/";

  if (_root != "") {
    ft_error(0, root, "root");
    return false;
  }
  // We need root path to start with "/"
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
