#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include "utils/server_utils.hpp"
#include "utils/parse_utils.hpp"
#include "utils/utils.hpp"
#include <arpa/inet.h>
#include <fstream>
#include <netinet/in.h>
#include <sys/types.h>
#include <algorithm>

#define MAX_LISTEN 1000

class Server {
public:
  typedef std::pair<std::string, std::string> host_port_type;

private:
  std::string _host;
  //std::string _server_name;
  std::vector<std::string> _server_names;
  std::string _root;
  std::string _index;
  int _client_max_body_size;
  std::vector<host_port_type> _endpoints;
  std::vector<int> _sockets;
  std::map<std::string, t_location> _location_list;

  bool parseOption(const int &, std::string &, std::stringstream &,
                   const std::string &);
  void addDefaultLocation();

public:
  Server();
  ~Server();

  std::string getHost() const;
  std::vector<std::string> getServerNames() const;
  std::string getRoot() const;
  std::string getIndex() const;
  int getBodySize() const;
  std::map<std::string, t_location> getLocationlist() const;
  std::vector<int> getSockets() const;
  std::vector<host_port_type> getEndpoints() const;

  bool setHost(const std::string &);
  bool addServerName(const std::string &);
  bool setRoot(std::string &);
  bool setIndex(const std::string &);
  bool setBodySize(const std::string &);
  bool addEndpoint(const std::string &);
  bool addLocation(std::stringstream &, std::string &value);

  bool parseServer(const std::string &, const std::string &,
                   std::vector<std::string> &);
  bool initServer();

  void handleRequest(struct t_request &, std::vector<std::string> &, bool &);
};

#endif
