#include "../../include/utils/utils.hpp"

/*
Because of the -std=c++98 flag, we can't use std::string::pop_back(),
std::stoi() and std::to_string() So I coded them here
*/
std::string ft_pop_back(std::string str) {
  std::string result = "";
  int pos = str.size() - 1;

  for (int i = 0; i < pos; i++)
    result += str[i];

  return result;
}

std::string ft_to_string(int nb) {
  std::string result = "", ch;

  while (nb > 0) {
    ch = nb % 10 + '0';
    ch += result;
    result = ch;
    nb /= 10;
  }
  return ch;
}

// A function to delete any white space before and after a line in the
// configuration file
std::string trim(const std::string &str) {
  std::string result;

  if (str.size() && str.find_first_not_of(" \t") != std::string::npos) {
    result = &str[str.find_first_not_of(" \t")];
    while (result[result.size() - 1] == ' ' ||
           result[result.size() - 1] == '\t')
      result = ft_pop_back(result);
  }
  return result;
}

int ft_stoi(std::string str) {
  int result = 0;
  int size = str.size();

  for (int i = 0; i < size; i++)
    result = result * 10 + str[i] - '0';

  return result;
}

void log(std::string line, std::string client, std::string server,
         std::string url, int type) {
  time_t tm = std::time(NULL);
  char *dt = ctime(&tm);
  std::string odt = ft_pop_back(dt);
  std::ofstream log_file;

  log_file.open("./webserv.log", std::ofstream::app);

  log_file << odt << " - ";

  switch (type) {
  case 0:
    log_file << line << "\n";
    break;
  case 1:
    log_file << client << ": " << line << "\n";
    break;
  case 2:
    log_file << server << ": Request (" << url << ") received from " << client
             << "\n";
    break;
  case 3:
    log_file << server << ": Reponse (" << &url[1] << ") sent to " << client
             << "\n";
    break;
  }

  log_file.close();
}

void sendText(t_request &request) {
  std::ifstream ifs(request.url.c_str());
  std::string html = "", buffer,
              extension = &request.url[request.url.find_last_of(".") + 1];
  // We start our response by the http header with the right code
  std::string result = "HTTP/1.1 ";

  result.append(request.code);
  result.append("\nContent-Type: text/");
  result.append(getContentType(extension));
  result.append("\nContent-Length: ");

  while (!ifs.eof()) {
    getline(ifs, buffer);
    html.append(buffer);
    html.append("\n");
  }
  ifs.close();
  result.append(ft_to_string(
      html.size())); // We append the size of the html page to the http response
  result.append("\n\n"); // The http response's header stops here
  result.append(html);   // The http reponse body (html page)

  if (DISPLAY_HTML)
    std::cout << result << std::endl;

  write(request.socket, result.c_str(), result.size());
}

void sendFile(t_request &request) {
  std::ifstream ifs(request.url.c_str(), std::ifstream::binary);
  std::string file = "", buffer,
              extension = &request.url[request.url.find_last_of(".") + 1];
  std::string result = "HTTP/1.1 200 OK\nContent-Type: ";

  result.append(getContentType(extension));
  result.append("\nContent-Length: ");

  while (!ifs.eof()) {
    getline(ifs, buffer);
    file.append(buffer);
    file.append("\n");
  }
  result.append(ft_to_string(file.size()));
  result.append("\n\n");
  result.append(file);

  write(request.socket, result.c_str(), result.size());

  ifs.close();
}

std::string getContentType(std::string extension) {
  if (extension == "css")
    return "css";
  else if (extension == "html" || extension == "htm")
    return "html";
  else if (extension == "ico")
    return ("image/x-icon");
  return "";
}
