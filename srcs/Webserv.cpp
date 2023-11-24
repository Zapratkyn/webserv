#include "../include/Webserv.hpp"

using namespace webserv_utils;

Webserv::Webserv(const std::string &conf_file)
    : _conf(conf_file) {
  _folder_list.push_back("/www/");
  _url_list.push_back("./stylesheet.css");
  _url_list.push_back("./favicon.ico");
  _url_list.push_back("./parentDirectory.png");
  _url_list.push_back("./directory.png");
  _url_list.push_back("./file.png");
  _url_list.push_back("./webPage.png");
  parseUrl("./www/", _url_list, _folder_list);
  if (DISPLAY_URL) {
    for (std::vector<std::string>::iterator it = _url_list.begin();
         it != _url_list.end(); it++)
      std::cout << *it << std::endl;
  }
}

Webserv::~Webserv() {
  // Since servers in the server list are dynamically allocated pointers, we
  // delete each server one by one in the destructor at the end of the program
  for (std::map<int, Server *>::iterator it = _server_list.begin();
       it != _server_list.end(); it++)
    delete it->second;
  for (std::vector<int>::iterator it = _listen_socket_list.begin();
       it != _listen_socket_list.end(); it++)
    close(*it);
  for (std::map<int, struct t_request>::iterator it = _request_list.begin();
       it != _request_list.end(); it++)
    close(it->first);
  return;
}

/*
- Isolate every server block in the conf file using the brackets
- Find the server's name in the block we just isolated and use it to add an
entry in the server list (If the server has no name or his name is
'webserv_42(_)', we append a number to differienciate them.)
- Send the server block to a parsing function, in the server class so we can use
its attributes without getters
*/

static bool validFile(const std::string &file) {
  int pos = file.find_last_of('.');
  std::string extension = &file[pos];

  if (!pos || (extension != ".conf" && extension != ".cnf"))
    return false;

  std::ifstream ifs;

  ifs.open(file.c_str());
  if (ifs.fail())
    return false;
  ifs.close();
  return true;
}

void Webserv::parse() {
  if (!validFile(_conf))
    throw confFailureException();
  std::ifstream infile(_conf.c_str());
  std::string buffer, server_block, server_name;
  Server *server;
  std::vector<int> port_list;

  while (!infile.eof()) {
    getline(infile, buffer);
    if (buffer == "server {") {
      server_block = getServerBlock(infile);
      server = new Server;
      int server_id = server->getServerID();
      if (!server->parseServer(server_block, _folder_list)) {
        /*
        If an error occurs, the server will not be added to the webserv's list
        of servers Therefore, we need to delete it here to avoid leaks
        */
        delete server;
        throw confFailureException();
      }
      _server_list[server_id] = server;
    }
  }
  infile.close();
  if (DISPLAY_SERVERS)
    displayServers(_server_list);
}

void Webserv::init() {
  try {
    std::map<int, Server *>::const_iterator it;
    for (it = _server_list.begin(); it != _server_list.end(); ++it) {
      it->second->initServer(_sock_addrs, _listen_socket_list);
    }
  } catch (const std::exception &e) {
    throw;
  }
}

void Webserv::run() {
  listenLog(_server_list);
  log("Webserv started", "", "", "", 0);
}



//void Webserv::startListen() {
//  listenLog(_server_list);
//  log("Webserv started", "", "", "", 0);
//
//  /*
//  Select() needs the biggest fd + 1 from all the fd_sets
//  Since fd 0, 1 and 2 are already taken (STD_IN and STD_OUT and STD_ERR), our
//  list begins at 3 Therefore, max_fds = total_number_of_sockets + STD_IN +
//  STD_OUT + STD_ERR
//  */
//  int max_fds = _listen_socket_list.size() + 3, step = 1, select_return;
//  fd_set readfds, writefds;
//  bool kill = false;
//  struct timespec ts;
//  sigset_t sigmask;
//
//  ts.tv_sec = 2;
//
//  while (!kill) {
//    if (step == 1) {
//      FD_ZERO(&readfds);
//      for (std::vector<int>::iterator it = _listen_socket_list.begin();
//           it != _listen_socket_list.end(); it++)
//        FD_SET(*it, &readfds);
//    }
//    /*
//    Select blocks until a new request is received
//    It then sets the request receiving sockets to 1 and unblocks
//    acceptNewConnections() will create new sockets and add them to the readfds
//    fd_set After a 2nd select(), we read from the newly created sockets and
//    parse the requests' headers and bodies Go through select() again before
//    handling the stacked requests Then handle all the stacked requests Reset the
//    readfds with the listening sockets (see above) Go through the whole process
//    again
//    */
//    select_return = pselect(max_fds, &readfds, &writefds, NULL, &ts, &sigmask);
//    if (select_return == 0) {
//      if (!_request_list.empty()) {
//        for (std::map<int, t_request>::iterator it = _request_list.begin();
//             it != _request_list.end(); it++) {
//          it->second.url = "./www/errors/500.html";
//          it->second.code = "500 Internal Server Error";
//          sendText(it->second);
//        }
//        _request_list.clear();
//      }
//      step = 1;
//      continue;
//    }
//    if (step == 1)
//      acceptNewConnections(max_fds, readfds);
//    else if (step == 2 && !_request_list.empty())
//      readRequests(readfds, writefds);
//    else if (step == 3 && !_request_list.empty())
//      sendRequests(kill, writefds, max_fds);
//    if (++step == 4)
//      step = 1;
//  }
//  FD_ZERO(&readfds);
//  FD_ZERO(&writefds);
//  log("Webserv stopped", "", "", "", 0);
//}


//void Webserv::acceptNewConnections(int &max_fds, fd_set &readfds) {
//  int new_socket;
//  struct t_request new_request;
//  std::vector<int> new_socket_list;
//
//  for (int socket = 3; socket < max_fds; socket++) {
//    if (FD_ISSET(socket, &readfds)) {
//      while (true) {
//        initRequest(new_request);
//        new_socket = accept(socket, (sockaddr *)&_socketAddr, &_socketAddrLen);
//        if (new_socket < 0)
//          break;
//        new_request.server = getServer(_server_list, socket);
//        new_request.client = inet_ntoa(_socketAddr.sin_addr);
//        new_request.socket = new_socket;
//        _request_list[new_socket] = new_request;
//        max_fds++;
//        new_socket_list.push_back(new_socket);
//      }
//    }
//  }
//  // We keep only the new sockets to avoid having select() applied on the same
//  // sockets several times
//  FD_ZERO(&readfds);
//  for (std::vector<int>::iterator it = new_socket_list.begin();
//       it != new_socket_list.end(); it++)
//    FD_SET(*it, &readfds);
//}

//void Webserv::readRequests(fd_set &readfds, fd_set &writefds) {
//  for (std::map<int, t_request>::iterator it = _request_list.begin();
//       it != _request_list.end(); it++) {
//    if (FD_ISSET(it->first, &readfds)) {
//      try {
//        getRequest(_server_list[it->second.server]->getBodySize(), it->second);
//        FD_SET(it->first, &writefds);
//      } catch (const std::exception &e) {
//        log(e.what(), it->second.client, "", "", 1);
//      }
//    }
//  }
//  // We don't need to read the requesting sockets anymore
//  FD_ZERO(&readfds);
//}
//
//void Webserv::sendRequests(bool &kill, fd_set &writefds, int &max_fds) {
//  std::map<int, struct t_request>::iterator tmp;
//
//  for (std::map<int, struct t_request>::iterator it = _request_list.begin();
//       it != _request_list.end();) {
//    if (FD_ISSET(it->first, &writefds)) {
//      _server_list[it->second.server]->handleRequest(it->second, _url_list,
//                                                     kill);
//      if (kill)
//        break;
//      close(it->first); // Closes the socket so it can be used again later
//      tmp = it++; // If I erase an iterator while itering on a std::map, I get a
//                  // SEGFAULT
//      _request_list.erase(tmp);
//      max_fds--;
//    }
//  }
//  // We don't want select() to test this fd_set anymore
//  FD_ZERO(&writefds);
//  _request_list.clear();
//}