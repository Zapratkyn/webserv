/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: samunyan <samunyan@student.s19.be>         +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2023/11/16 14:28:31 by samunyan          #+#    #+#             */
/*   Updated: 2023/11/16 17:58:18 by samunyan         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <arpa/inet.h>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>

void printSocketAddress(struct sockaddr_in &_socketAddr) {
  char s[INET_ADDRSTRLEN] = {};

  inet_ntop(AF_INET, (void *)&_socketAddr.sin_addr, s, INET_ADDRSTRLEN);
  std::cout << s << ":" << ntohs(_socketAddr.sin_port) << std::endl;
}

bool setListenSocketAddress(const std::string &IPAddress,
                            const std::string &portNum,
                            struct sockaddr_in *_socketAddr) {
  struct addrinfo hints = {};
  struct addrinfo *res = NULL;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICSERV;

  int status = getaddrinfo(IPAddress.c_str(), portNum.c_str(), &hints, &res);
  if (status == 0 && res != NULL) {
    *_socketAddr = *(struct sockaddr_in *)res->ai_addr;
  } else {
    std::cerr << "Error: getaddrinfo: " << gai_strerror(status)
              << " for IP address \"" + IPAddress + "\" and port \"" + portNum +
                     "\""
              << std::endl;
  }
  freeaddrinfo(res);
  return (status == 0 ? true : false);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: ./address <IP address or hostname> <port number>"
              << std::endl;
    return 1;
  }
  struct sockaddr_in _socketAddr = {};
  if (!setListenSocketAddress(argv[1], argv[2], &_socketAddr)) {
    return 1;
  } else {
    printSocketAddress(_socketAddr);
  }
  return 0;
}