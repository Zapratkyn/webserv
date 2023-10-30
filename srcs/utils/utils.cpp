#include "../../include/utils/utils.hpp"

/*
Because of the -std=c++98 flag, we can't use pop_back(), std::stoi() and std::to_string()
So I coded them here
*/
std::string ft_pop_back(std::string str)
{
	std::string result = "";
	int pos = str.size() - 1;

	for (int i = 0; i < pos; i++)
    	result += str[i];

	return result;
}

std::string ft_to_string(int nb)
{
	std::string result = "", ch;

	while (nb > 0)
	{
		ch = nb % 10 + '0';
		ch += result;
    	result = ch;
		nb /= 10;
	}
    return ch;
}

// A function to delete any white space before and after a line in the configuration file
std::string trim(const std::string &str)
{
    std::string result;

    if (str.size())
    {
        result = &str[str.find_first_not_of(" \t")];
        while (result[result.size() - 1] == ' ' || result[result.size() - 1] == '\t')
            result = ft_pop_back(result);
    }
    return result;
}

int ft_stoi(std::string str)
{
	int result = 0;
	int size = str.size();

	for (int i = 0; i < size; i++)
		result = result * 10 + str[i] - '0';

	return result;
}