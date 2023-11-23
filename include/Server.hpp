#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include "utils/server_utils.hpp"
#include "utils/utils.hpp"
#include <netdb.h>

class Server {
public:
  typedef std::pair<std::string, std::string> host_port_type;

private:
  std::string _host;
  std::string
      _server_name; // replaced by std::vector<std::string> _server_names
  std::vector<std::string> _server_names; // replaces std::string _server_name;
  std::string _root;
  std::string _index;
  int _client_max_body_size;
  std::vector<int>
      _ports; // replaced by std::vector<host_port_type> _endpoints;
  std::vector<host_port_type> _endpoints; // replaces std::vector<int> _ports;
  std::vector<int> _sockets;
  std::map<std::string, t_location> _location_list;

  bool parseOption(const int &, std::string &, std::stringstream &,
                   const std::string &);
  void addDefaultLocation();

public:
  Server();
  ~Server();

  std::string getHost() const;
  std::string getServerName()
      const; // replaced by std::vector<std::string> getServerNames() const;
  std::vector<std::string>
  getServerNames() const; // replaces std::string getServerName() const;
  std::string getRoot() const;
  std::string getIndex() const;
  int getBodySize() const;
  std::vector<int> getPorts()
      const; // replaced by std::vector<host_port_type> getEndpoints() const;
  std::vector<host_port_type>
  getEndpoints() const; // replaces std::vector<int> getPorts() const;
  std::map<std::string, t_location> getLocationlist() const;
  std::vector<int> getSockets() const;

  bool setHost(const std::string &);
  bool
  setServerName(const std::string
                    &); // replaced by  bool addServerName(const std::string &);
  bool addServerName(
      const std::string &); // replaces  bool setServerName(const std::string&);
  bool setRoot(std::string &);
  bool setIndex(const std::string &);
  bool setBodySize(const std::string &);
  bool addPort(
      const std::string &,
      std::vector<int> &); // replaced by bool addEndpoint(const std::string &);
  bool addEndpoint(const std::string &); // replaces  bool addPort(const
                                         // std::string &, std::vector<int> &);
  bool addLocation(std::stringstream &, std::string &value);
  void addSocket(int);

  bool parseServer(const std::string &, const std::string &,
                   std::vector<std::string> &);
  bool initServer();

  void handleRequest(struct t_request &, std::vector<std::string> &, bool &);
};

#endif
