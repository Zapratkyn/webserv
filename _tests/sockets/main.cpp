#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <iostream>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8084
#define BACKLOG 10 // max number of connection requests in queue

int oneConnection();
int multiplexingWithSelect();

int initServer(const struct sockaddr *addr, socklen_t addrlen, int backlog);
void acceptNewConnection(int server_fd, fd_set *all_read, int *fd_max);
void getRequest(int client_fd, fd_set *all_read, fd_set *all_write,
                int *fd_max);
void sendResponse(int client_fd, fd_set *all_read, fd_set *all_write,
                  int *fd_max);

////////////////////////////////////////////////////////////////////////////////

int main() { return (multiplexingWithSelect()); }

////////////////////////////////////////////////////////////////////////////////

int oneConnection() {

  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  addr.sin_port = htons(PORT);

  int server_fd = initServer((struct sockaddr *)&addr, sizeof addr, BACKLOG);
  if (server_fd < 0) {
    std::cerr << "Error: " << strerror(errno) << std::endl;
    return (EXIT_FAILURE);
  }

  int client_fd = accept(server_fd, NULL, NULL);
  if (client_fd < 0) {
    std::cerr << "Error: " << strerror(errno) << std::endl;
    close(server_fd);
    return (EXIT_FAILURE);
  } else {
    std::cout << "Accepted new connection on client socket fd: " << client_fd
              << std::endl;
  }

  char buffer[BUFSIZ];
  while (true) {
    std::cout << "Reading client socket " << client_fd << std::endl;
    ssize_t bytes_received = recv(client_fd, buffer, BUFSIZ, 0);
    if (bytes_received == 0) {
      std::cout << "Client socket " << client_fd << ": closed connection"
                << std::endl;
      break;
    } else if (bytes_received < 0) {
      std::cerr << "Error: " << strerror(errno) << std::endl;
      close(client_fd);
      close(server_fd);
      return (EXIT_FAILURE);
    } else {
      std::cout << "Message received from client socket fd:" << client_fd
                << ": " << buffer << std::endl;
      std::string msg("HTTP/1.1 200 OK\nContent-Type: "
                      "text/plain\nContent-Length: 12\n\nHello world!");
      ssize_t bytes_sent = send(client_fd, msg.c_str(), msg.size(), 0);
      if (bytes_sent < 0) {
        std::cerr << "Error: " << strerror(errno) << std::endl;
        close(client_fd);
        close(server_fd);
        return (EXIT_FAILURE);
      } else if (bytes_sent == static_cast<ssize_t>(msg.size())) {
        std::cout << "Sent full message to client socket fd: " << client_fd
                  << ": " << msg << std::endl;
      } else {
        std::cout << "Sent full message to client socket fd: " << client_fd
                  << ": " << bytes_sent << " bytes_sent" << std::endl;
      }
    }
  }
  close(client_fd);
  close(server_fd);
  return (EXIT_SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////

int multiplexingWithSelect() {

  // For server sockets
  struct sockaddr_in addr = {};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_ANY);
  addr.sin_port = htons(PORT);

  // Create server socket and listen to port via socket
  int server_fd = initServer((struct sockaddr *)&addr, sizeof addr, BACKLOG);
  fcntl(server_fd, F_SETFL, O_NONBLOCK);

  // To monitor socket fds:
  fd_set all_read;  // Set for all sockets connected to server
  fd_set all_write; // Set for all sockets connected to server
  fd_set read_fds;  // Temporary set for select()
  fd_set write_fds; // Temporary set for select()
  int fd_max;       // Highest socket fd
  struct timeval timer = {};

  // Prepare socket sets for select()
  FD_ZERO(&all_read);
  FD_ZERO(&all_write);
  FD_ZERO(&read_fds);
  FD_ZERO(&write_fds);
  FD_SET(server_fd, &all_read); // Add listener socket to set
  fd_max = server_fd;           // Highest fd is necessarily our socket

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
      std::cout << "Server is waiting ..." << std::endl;
      continue;
    }
    // Loop over our sockets
    for (int i = 0; i <= fd_max; ++i) {
      if (FD_ISSET(i, &read_fds)) {
        // Fd i is not a socket to monitor
        // stop here and continue the loop
        if (i == server_fd) {
          // Socket is our server's listener socket
          acceptNewConnection(server_fd, &all_read, &fd_max);
        } else {
          getRequest(i, &all_read, &all_write, &fd_max);
        }
      } else if (FD_ISSET(i, &write_fds) && i != server_fd) {
        sendResponse(i, &all_read, &all_write, &fd_max);
      }
    }
  }
}

///////////////////////////////////////////////////////////////////////////////

int initServer(const struct sockaddr *addr, socklen_t addrlen, int backlog) {
  int fd;
  int reuse = true;
  int err;
  if ((fd = socket(addr->sa_family, SOCK_STREAM, 0)) < 0) {
    return (-1);
  } else {
    std::cout << "Created server socket fd: " << fd << std::endl;
  }
  if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
    err = errno;
    close(fd);
    errno = err;
    return (-1);
  }
  if (bind(fd, addr, addrlen) < 0) {
    err = errno;
    close(fd);
    errno = err;
    return (-1);
  } else {
    char str[INET_ADDRSTRLEN] = {};
    std::cout << "Bound socket_fd to address: "
              << inet_ntop(addr->sa_family,
                           &((struct sockaddr_in *)addr)->sin_addr, str,
                           INET_ADDRSTRLEN)
              << " and port: " << ntohs(((struct sockaddr_in *)addr)->sin_port)
              << std::endl;
  }
  if (listen(fd, backlog) < 0) {
    err = errno;
    close(fd);
    errno = err;
    return (-1);
  } else {
    std::cout << "Listening on port: "
              << ntohs(((struct sockaddr_in *)addr)->sin_port) << std::endl;
  }
  return (fd);
}

void acceptNewConnection(int server_fd, fd_set *all_read, int *fd_max) {
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

void getRequest(int client_fd, fd_set *all_read, fd_set *all_write,
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

void sendResponse(int client_fd, fd_set *all_read, fd_set *all_write,
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
