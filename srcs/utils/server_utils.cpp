#include "../../include/utils/server_utils.hpp"

namespace server_utils {

bool allowedMethod(std::string &method, std::vector<std::string> &list) {
  for (std::vector<std::string>::iterator it = list.begin(); it != list.end();
       it++) {
    if (*it == method)
      return true;
  }
  return false;
}

bool validMethod(std::string &method) {
  if (method != "GET" && method != "DELETE" && method != "POST" &&
      method != "HEAD" && method != "PUT" && method != "CONNECT" &&
      method != "OPTIONS" && method != "TRACE" && method != "PATCH")
    return false;
  return true;
}

void setRequest(t_request &request, bool &kill) {
  std::string first_line =
      request.header.substr(0, request.header.find_first_of("\n"));

  request.method = first_line.substr(0, first_line.find_first_of(" \t"));
  if (!validMethod(request.method)) {
    request.url = "./www/errors/400.html";
    request.code = "400 Bad Request";
    sendUrl(request);
    throw invalidMethodException();
  }
  first_line = &first_line[first_line.find_first_of(" \t")];
  first_line = &first_line[first_line.find_first_not_of(" \t")];
  request.location = first_line.substr(0, first_line.find_first_of(" \t"));

  if (DISPLAY_METHOD_AND_LOCATION) {
    std::cout << "Method = " << request.method << std::endl;
    std::cout << "Location = " << request.location << std::endl;
  }

  if (request.location == "/kill") {
    kill = true;
    request.url = "./kill.html";
    sendUrl(request);
  }
}

void checkUrl(struct t_request &request, std::vector<std::string> &url_list) {
  std::string dot = ".";

  log("", request.client, request.server, request.location, 2);

  std::string extension = &request.location[request.location.find_last_of(".")];

  if (extension == ".html" || extension == ".htm" || extension == ".php") {
    request.is_url = true;
    request.location = dot.append(request.location);
    for (std::vector<std::string>::iterator it = url_list.begin();
         it != url_list.end(); it++) {
      /*
      If the requested url exists in the Webserv's list, we provide the page
      If not, we provide the 404 error page
      */
      if (*it == request.location) {
        request.url = *it;
        sendUrl(request);
        return;
      }
    }
    request.url = "./www/errors/404.html";
    request.code = "404 Not found";
    sendUrl(request);
  }

  if (request.location[request.location.size() - 1] != '/')
    request.location.append("/");
}

void checkLocation(struct t_request &request,
                   std::map<std::string, struct t_location> &location_list) {
  std::string dot = ".";

  for (std::map<std::string, struct t_location>::iterator it =
           location_list.begin();
       it != location_list.end(); it++) {
    if (request.location == it->first) {
      if (it->second.index != "") {
        request.url = it->second.root.append(it->second.index);
        request.url = dot.append(request.url);
        sendUrl(request);
      } else if (it->second.autoindex == "on")
        sendTable(request, it->second.root);
    }
  }
  request.url = "./www/errors/404.html";
  request.code = "404 Not found";
  sendUrl(request);
}

void sendTable(struct t_request &request, std::string root) {
  std::ifstream ifs("./dir.html");
  std::string html = "", buffer;
  std::string result =
      "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";

  while (!ifs.eof()) {
    getline(ifs, buffer);
    html.append(buffer);
    html.append("\n");
  }
  ifs.close();

  html.insert(html.find("</title>"), request.server);
  html.insert(html.find("</h2>"), request.location);

  if (root != "/www/")
    addParentDirectory(html, root);

  addLinkList(html, root);

  result.append(ft_to_string(
      html.size())); // We append the size of the html page to the http response
  result.append("\n\n"); // The http response's header stops here
  result.append(html);   // The http reponse body (html page)

  write(request.socket, result.c_str(), result.size());
}

void addParentDirectory(std::string &html, std::string location) {
  int spot = html.rfind("</table>");
  std::string loc = location.substr(0, location.find_last_of("/"));

  loc = loc.substr(0, loc.find_last_of("/"));

  html.insert(spot, "\t<tr>\n\t\t<td></td>\n\t\t<td><a href=");
  spot = html.rfind("</table>");
  html.insert(spot++, 1, '"');
  html.insert(spot, loc);
  spot = html.rfind("</table>");
  html.insert(spot++, 1, '"');
  html.insert(spot,
              ">Parent directory</a></td>\n\t\t<td>Directory</td>\n\t</tr>\n");
}

void addLinkList(std::string &html, std::string location) {
  DIR *dir;
  struct dirent *file;
  std::string file_name, url_copy, extension, dot = ".";
  int spot;

  dir = opendir(dot.append(location).c_str());
  file = readdir(dir);

  while (file) {
    file_name = file->d_name;
    if (file_name == "." || file_name == "..") {
      file = readdir(dir);
      continue;
    }
    spot = html.rfind("</table>");
    html.insert(spot, "\t<tr>\n\t\t<td></td>\n\t\t<td><a href=");
    spot = html.rfind("</table>");
    html.insert(spot++, 1, '"');
    url_copy = location;
    url_copy.append(file_name);
    html.insert(spot, url_copy);
    spot = html.rfind("</table>");
    html.insert(spot++, 1, '"');
    html.insert(spot++, 1, '>');
    html.insert(spot, file_name);
    spot = html.rfind("</table>");
    html.insert(spot, "</a></td>\n\t\t<td>");
    spot = html.rfind("</table>");
    extension = &url_copy[url_copy.find_last_of(".")];
    if (extension == ".html" || extension == ".htm" || extension == ".php")
      html.insert(spot, "Web page");
    else if (extension == ".file")
      html.insert(spot, "File");
    else
      html.insert(spot, "Directory");
    spot = html.rfind("</table>");
    html.insert(spot, "</td>\n\t</tr>\n");
    file = readdir(dir);
  }
  closedir(dir);
}

int setSocketAddress(const std::string &ip_address, const std::string &port_num,
                     struct sockaddr_in *socket_addr) {
  struct addrinfo hints = {};
  struct addrinfo *res = NULL;

  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_NUMERICSERV;

  int status = getaddrinfo(ip_address.c_str(), port_num.c_str(), &hints, &res);
  if (status == 0 && res != NULL)
    *socket_addr = *(struct sockaddr_in *)res->ai_addr;
  freeaddrinfo(res);
  return (status);
}

int getSocketAddress(int socket, struct sockaddr_in *addr) {
  socklen_t len = sizeof *addr;
  return (getsockname(socket, (struct sockaddr *)addr, &len));
}

void printSocketAddress(struct sockaddr_in &_socketAddr) {
  char s[INET_ADDRSTRLEN] = {};

  inet_ntop(AF_INET, (void *)&_socketAddr.sin_addr, s, INET_ADDRSTRLEN);
  std::cout << s << ":" << ntohs(_socketAddr.sin_port) << std::endl;
}

}; // namespace server_utils
