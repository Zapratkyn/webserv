#include <iostream>
#include <string>
#include <fstream>
#include <sstream>

int main()
{
	std::ifstream ifs("./www/server00/assets/favicon.ico", std::ifstream::binary | std::ifstream::ate);

	std::cout << ifs.gcount() << std::endl;
	// std::stringstream ss;
	// std::string str;

	// ss << ifs.rdbuf();
	// str = ss.str();

	// std::cout << str << std::endl;

	// std::cout << "==========" << std::endl;

	// std::cout << &str[50] << std::endl;

	return 0;
}