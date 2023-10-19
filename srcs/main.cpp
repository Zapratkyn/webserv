#include "../include/Webserv.hpp"

bool valid_file(const std::string &);

int main(int argc, char **argv)
{
	if (argc > 2)
	{
		std::cerr << "ERROR\nExpected : ./webserv [conf file]" << std::endl;
		return EXIT_FAILURE;
	}
	
	std::string conf_file = "conf/default.conf";

	
	if (argc == 2)
	{
		if (!valid_file(argv[1]))
		{
			std::cerr << "ERROR\nInvalid configuration file" << std::endl;
			return EXIT_FAILURE;
		}
		conf_file = argv[1];
	}

	Webserv webServer(conf_file);

	try
	{
		webServer.parseConf();
		// server.startServer();
		// server.startListen();
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() <<std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}

bool valid_file(const std::string &file)
{
	if (file.find('.') == file.npos || file.find('.') == 0)
	{
		std::cerr << "ERROR\nInvalid configuration file" << std::endl;
		return false;
	}
	if (file.substr(file.find_last_of('.'), file.size()) != "conf")
	{
		std::cerr << "ERROR\nInvalid configuration file" << std::endl;
		return false;
	}
	if (std::ifstream(file))
		return true;
	std::cerr << "ERROR\nThe configuration file does not exist" << std::endl;
	return false;
}