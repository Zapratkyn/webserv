#include <iostream>
#include <fstream>
#include <sstream>

int main(int argc, char **argv, char **env)
{
	std::ofstream outfile;
	std::stringstream ss;
	std::string str(argv[0]);

	std::cout << "I'm in the CGI !!!\n" << std::endl;

	return 200;
}