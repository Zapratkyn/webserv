#include "../include/Webserv.hpp"

bool valid_file(const std::string &);

int main(int argc, char **argv)
{
	if (argc > 2)
	{
		std::cerr << "ERROR\nExpected : ./webserv [configuration file path/name]" << std::endl;
		return EXIT_FAILURE;
	}
	
	std::string conf_file = "./conf/default.conf";
	std::string conf_folder = "./conf/";
	
	if (argc == 2)
	{
		conf_file = argv[1];
		if (conf_file.size() >= 8 && conf_file.substr(0, 7) != conf_folder)
			conf_file = conf_folder.append(conf_file);
		if (!valid_file(conf_file)) // Make sure the configuration file exists and has a correct extension (".conf" / "cnf")
		{
			std::cerr << "ERROR\nInvalid configuration file" << std::endl;
			return EXIT_FAILURE;
		}
	}

	Webserv webServer(conf_file);

	try
	{
		webServer.parseConf(); // Using the configuration file to fill the Webserv's list of servers
		webServer.display_servers();
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
	int pos = file.find_last_of('.');
	std::string extension = &file[pos];

	if (!pos || (extension != ".conf" && extension != ".cnf"))
		return false;

	std::ifstream ifs;

	ifs.open(file);
	if (ifs.fail())
		return false;
	ifs.close();
	return true;
}