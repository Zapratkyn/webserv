#include "../include/Webserv.hpp"

int main(int argc, char **argv) {
  if (argc > 2) {
    std::cerr << "ERROR\nExpected : ./webserv [configuration file path]"
              << std::endl;
    return EXIT_FAILURE;
  }

  std::string conf_file;
  (argc == 2) ? conf_file = argv[1] : conf_file = "./conf/default.conf";

  Webserv webserver(conf_file);

  try {
    webserver.parse();
   webserver.init();
    webserver.startListen(); // Actual main loop
  } catch (const std::exception &e) {
    std::cerr << "Error : " << e.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}