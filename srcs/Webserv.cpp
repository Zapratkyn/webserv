#include "../include/Webserv.hpp"

using namespace webserv_utils;

Webserv::Webserv(const std::string &conf_file) : _conf(conf_file) {
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

void Webserv::_parse() {
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

void Webserv::_init() {
  std::map<int, Server *>::const_iterator it;
  for (it = _server_list.begin(); it != _server_list.end(); ++it) {
    it->second->initServer(_sock_addrs, _listen_socket_list);
  }
}

bool Webserv::_isListeningSocket(int fd) {
  std::vector<int>::const_iterator it = find (_listen_socket_list.begin(), _listen_socket_list.end(), fd);
  return (it != _listen_socket_list.end());
}

void Webserv::run() {
  _parse();
  _init();
  listenLog(_server_list);
  log("Webserv started", "", "", "", 0);

  // To monitor socket fds:
  fd_set all_read;  // Set for all sockets connected to server
  fd_set all_write; // Set for all sockets connected to server
  fd_set read_fds;  // Temporary set for select()
  fd_set write_fds; // Temporary set for select()
  int fd_max;
  struct timeval timer = {};

  // Prepare socket sets for select()
  FD_ZERO(&all_read);
  FD_ZERO(&all_write);
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  // Add listener sockets to set
  std::vector<int>::const_iterator it = _listen_socket_list.begin();
  for (; it != _listen_socket_list.end(); ++it) {
    FD_SET(*it, &all_read);
  }
  // Set highest fd
  fd_max =
      *std::max_element(_listen_socket_list.begin(), _listen_socket_list.end());

  while (true) {
    // Copy all socket set since select() will modify monitored set
    FD_COPY(&all_read, &read_fds);
    FD_COPY(&all_write, &write_fds);
    // 2 second timeout for select()
    timer.tv_sec = 2;
    timer.tv_usec = 0;
    // Monitor sockets ready for reading
    int status = select(fd_max + 1, &read_fds, &write_fds, NULL, &timer);
    if (status < 0) {
      std::cerr << "Error: " << strerror(errno) << std::endl;
      continue;
    } else if (status == 0) {
      // No socket fd is ready to read or write
      //std::cout << "Server is waiting ..." << std::endl;
      continue;
    }
    // Loop over our sockets
    for (int i = 0; i <= fd_max; ++i) {
      if (FD_ISSET(i, &read_fds)) {
        if (_isListeningSocket(i)) {
          // Socket is a server's listener socket
          _acceptNewConnection(i, &all_read, &fd_max);
        } else {
          _getRequest(i, &all_read, &all_write, &fd_max);
        }
      } else if (FD_ISSET(i, &write_fds) && !_isListeningSocket(i)) {
        _sendResponse(i, &all_read, &all_write, &fd_max);
      }
    }
  }
}

void Webserv::_acceptNewConnection(int server_fd, fd_set *all_read, int *fd_max) {
  int client_fd = accept(server_fd, NULL, NULL);
  if (client_fd < 0) {
    std::cerr << "Error: " << strerror(errno) << std::endl;
  } else {
    std::cout << "Accepted new connection on client socket fd: " << client_fd
              << std::endl;
    fcntl(client_fd, F_SETFL, O_NONBLOCK);
    FD_SET(client_fd, all_read); // Add the new client socket to the set
    if (client_fd > *fd_max)
      *fd_max = client_fd; // Update the highest socket
  }
}

void Webserv::_getRequest(int client_fd, fd_set *all_read, fd_set *all_write,
                int *fd_max) {
  char buffer[BUFSIZ] = {};
  std::cout << "Reading client socket " << client_fd << std::endl;
  ssize_t bytes_received = recv(client_fd, buffer, BUFSIZ, 0);
  if (bytes_received == 0) {
    std::cout << "Client socket " << client_fd << ": closed connection"
              << std::endl;
    close(client_fd);
    FD_CLR(client_fd, all_read); // Remove socket from the set
    if (client_fd == *fd_max)
      (*fd_max)--; // Update the highest socket
  } else if (bytes_received < 0) {
    std::cerr << "Error: " << strerror(errno) << std::endl;
    close(client_fd);
    FD_CLR(client_fd, all_read); // Remove socket from the set
    if (client_fd == *fd_max)
      (*fd_max)--; // Update the highest socket
  } else {
    std::cout << "Message received from client socket fd: " << client_fd
              << ":\n"
              << buffer << std::endl;
    FD_CLR(client_fd, all_read); // Remove socket from the set
    FD_SET(client_fd, all_write);
  }
}

void Webserv::_sendResponse(int client_fd, fd_set *all_read, fd_set *all_write,
                  int *fd_max) {
  std::string msg("HTTP/1.1 200 OK\r\nContent-Type: "
                  "text/plain\r\nContent-Length: 12\r\n\nHello world!");
  ssize_t bytes_sent = send(client_fd, msg.c_str(), msg.size(), 0);
  if (bytes_sent < 0) {
    std::cerr << "Error: " << strerror(errno) << std::endl;
    close(client_fd);
    FD_CLR(client_fd, all_write); // Remove socket from the set
    if (client_fd == *fd_max)
      (*fd_max)--; // Update the highest socket
  } else if (bytes_sent == static_cast<ssize_t>(msg.size())) {
    std::cout << "Sent full message to client socket fd: " << client_fd << ":\n"
              << msg << std::endl;
    FD_CLR(client_fd, all_write); // Remove socket from the set
    FD_SET(client_fd, all_read);
  } else {
    std::cout << "Sent partial message to client socket fd: " << client_fd
              << ":\n"
              << bytes_sent << " bytes_sent" << std::endl;
  }
}

