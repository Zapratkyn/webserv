#include <iostream>
#include <string>

int main()
{
	std::string str = "Salut les amis", str2 = "super potes que j'adore";

	str.replace(str.find("amis"), str2.size(), str2);

	std::cout << str << std::endl;

	return 0;
}