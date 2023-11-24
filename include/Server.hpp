#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include "utils/server_utils.hpp"
#include "utils/utils.hpp"
#include <algorithm>
#include <netdb.h>

class Server {
public:
  typedef std::pair<std::string, std::string> host_port_type;
protected:
  static int ID;
private:
  int _serverID;
  std::vector<std::string> _server_names; // replaces std::string _server_name;
  std::string _root;
  std::string _index;
  int _client_max_body_size;
  std::vector<int>
      _ports; // replaced by std::vector<host_port_type> _endpoints;
  std::vector<host_port_type> _endpoints; // replaces std::vector<int> _ports;
  std::vector<int> _sockets;
  std::vector<struct sockaddr_in> _sock_addrs;
  std::map<std::string, t_location> _location_list;

  bool parseOption(const int &, std::string &, std::stringstream &);
  void addDefaultLocation();

public:
  Server();
  ~Server();

  int getServerID() const;
  std::vector<std::string> getServerNames() const;
  std::string getRoot() const;
  std::string getIndex() const;
  int getBodySize() const;
  std::vector<int> getPorts() const;
  std::vector<host_port_type> getEndpoints() const;
  std::map<std::string, t_location> getLocationlist() const;
  std::vector<int> getSockets() const;
  std::vector<struct sockaddr_in> getSocketAddresses() const;

  bool addServerName(const std::string &);
  bool setRoot(std::string &);
  bool setIndex(const std::string &);
  bool setBodySize(const std::string &);
  bool addEndpoint(const std::string &);
  bool addLocation(std::stringstream &, std::string &value);
  void addSocket(int);

  bool initServer(std::vector<struct sockaddr_in> &webserv_sock_addrs,
                  std::vector<int> &webserv_sockets);

  bool parseServer(const std::string &server_block, std::vector<std::string> &folder_list);

  void handleRequest(struct t_request &, std::vector<std::string> &, bool &);
};

#endif
