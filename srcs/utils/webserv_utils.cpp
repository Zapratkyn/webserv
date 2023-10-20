#include "../../include/utils/webserv_utils.hpp"

namespace webserv_utils {

// A function to delete any white space before and after a line in the configuration file
	std::string trim(const std::string &str)
	{
	    std::string result;

	    if (str.size())
	    {
	        result = &str[str.find_first_not_of(" \t")];
	        while (result.back() == ' ' || result.back() == '\t')
	            result.pop_back();
	    }
	    return result;
	}

	std::string getServerName(const std::string &server_block)
	{
		std::stringstream 	ifs(server_block);
		std::string 		buffer, name, result, default_name = "webserv_42";

		while (!ifs.eof())
		{
			getline(ifs, buffer);
			name = buffer.substr(0, buffer.find_first_of(" \t"));
			if (name == "server_name")
			{
	            buffer = &buffer[buffer.find_first_of(" \t")];
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

};