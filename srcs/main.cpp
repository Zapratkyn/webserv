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
	std::string conf_folder = "conf/";
	
	if (argc == 2)
	{
		conf_file = argv[1];
		if (conf_file.size() >= 6 && conf_file.substr(0, 5) != conf_folder)
			conf_file = conf_folder.append(conf_file);
		if (!valid_file(conf_file)) // Make sure the configuration file exists and has the correct extension (".conf")
		{
			std::cerr << "ERROR\nInvalid configuration file" << std::endl;
			return EXIT_FAILURE;
		}
	}

	Webserv webServer(conf_file);

	try
	{
		webServer.parseConf(); // Using the configuration file to fill the Webserv's list of servers
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
	if (!file.find('.') || file.find('.') == 0)
		return false;
	
	if (&file[file.find_last_of('.')] != ".conf")
		return false;

	std::ifstream ifs;

	ifs.open(file);
	if (ifs.fail())
		return false;
	ifs.close();
	return true;
}