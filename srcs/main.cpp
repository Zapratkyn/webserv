#include "../include/Webserv.hpp"

bool valid_file(
    const std::string &); // I like to keep the main() function on top :)

int main(int argc, char **argv) {
  if (argc > 2) {
    std::cerr << "ERROR\nExpected : ./webserv [configuration file path]"
              << std::endl;
    return EXIT_FAILURE;
  }

  std::string conf_file = "./conf/default.conf";

  if (argc == 2) {
    conf_file = argv[1];
    if (!valid_file(conf_file)) // Make sure the configuration file exists and
                                // has a correct extension (".conf" / "cnf")
    {
      std::cerr << "ERROR\nInvalid configuration file" << std::endl;
      return EXIT_FAILURE;
    }
  }

  Webserv webserver(conf_file);

  try {
    webserver.parseConf(); // Using the configuration file to fill the Webserv's
                           // list of servers
    webserver.startServer(); // Create the listening sockets
    webserver.startListen(); // Actual main loop
  } catch (const std::exception &e) {
    std::cerr << e.what() << std::endl;
    return EXIT_FAILURE;
  }

  // For an unknown reason, if I don't display something here, I get a SegFault
  // error message...
  std::cout << "Webserv stopped\n" << std::endl;

  return EXIT_SUCCESS;
}

bool valid_file(const std::string &file) {
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
