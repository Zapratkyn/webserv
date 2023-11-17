#ifndef __WEBSERV_HPP__
#define __WEBSERV_HPP__

#include "utils/parse_utils.hpp"
#include "utils/utils.hpp"
#include "utils/webserv_utils.hpp"
#include <cstdlib>
#include <fcntl.h>

#define DISPLAY_URL true
#define DISPLAY_SERVERS true

#define MAX_LISTEN 1000

class Webserv {

private:
  std::vector<int> _listen_socket_list;
  std::map<int, struct t_request> _request_list;
  struct sockaddr_in _socketAddr;
  unsigned int _socketAddrLen;
  // struct timeval						_tv;
  std::vector<std::string> _url_list;
  std::vector<std::string> _folder_list;

  void acceptNewConnections(int &, fd_set &);
  void readRequests(fd_set &, fd_set &);
  void sendRequests(bool &, fd_set &);

  std::string _conf;
  std::map<std::string, Server *> _server_list;

public:
  Webserv(const std::string &);
  ~Webserv();
  void run();
  void init();
  void parse();

  class openSocketException : public std::exception {
  public:
    virtual const char *what() const throw() {
      return "ERROR\nCouldn't open socket";
    }
  };
  class bindException : public std::exception {
  public:
    virtual const char *what() const throw() {
      return "ERROR\nCouldn't bind socket";
    }
  };
  class listenException : public std::exception {
  public:
    virtual const char *what() const throw() {
      return "ERROR\nCouldn't start listening";
    }
  };
  class confFailureException : public std::exception {
  public:
    virtual const char *what() const throw() {
      return "Configuration failure. Program stopped.";
    }
  };
  class logError : public std::exception {
  public:
    virtual const char *what() const throw() { return "LOG ERROR."; }
  };
};

#endif