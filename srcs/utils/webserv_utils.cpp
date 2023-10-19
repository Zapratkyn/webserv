#include "../../include/utils/webserv_utils.hpp"

using namespace webserv_utils;

std::string &trim(const std::string &str)
{
    std::string result = "";
    int pos;

    if (str.size())
    {
        pos = str.find_first_not_of(" \t");
        result = &str[pos];
        pos = result.size() - 1;
        while (result[pos] == ' ' || result[pos] == '\t')
        {
            result.pop_back();
            pos--;
        }
    }
    return result;
}

std::string &getServerName(const std::string &server_block)
{
	std::ifstream 	ifs(server_block);
	std::string 	buffer, result, default_name = "webserv_42";
	int				pos = 0;

	while (!ifs.eof())
	{
		getline(ifs, buffer);
        buffer = &buffer[buffer.find_first_not_of(" \t")];
        pos = buffer.find_first_of(" \t");
		if (buffer.substr(0, pos) == "server_name")
		{
            buffer = &buffer[pos];
            buffer = &buffer[buffer.find_first_not_of(" \t")];
            if (!buffer[0])
                return default_name;
			result = buffer.substr(0);
			while (result.back() == ' ' || result.back() == '\t' || result.back() == ';' || result.back() == '}')
				result.pop_back();
			return (result);
		}
	}
	return default_name;
}