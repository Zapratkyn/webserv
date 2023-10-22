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

	std::string getServerName(const std::string &server_block, int &default_name_index, std::map<std::string, Server*> &server_list)
	{
		std::stringstream 	ifs(server_block);
		std::string 		buffer, name, result, default_name = "webserv_42_";

		while (!ifs.eof())
		{
			getline(ifs, buffer);
			name = buffer.substr(0, buffer.find_first_of(" \t"));
			if (name == "server_name")
			{
	            buffer = &buffer[buffer.find_first_of(" \t")];
	            buffer = &buffer[buffer.find_first_not_of(" \t")];
	            if (!buffer[0] || buffer.substr(0, 11) == "webserv_42;" || buffer.substr(0, 12) == "webserv_42_;")
				{
					default_name.append(std::to_string(default_name_index++));
	                return default_name;
				}
				result = buffer.substr(0);
				while (result.back() == ' ' || result.back() == '\t' || result.back() == ';' || result.back() == '}')
					result.pop_back();
				// If the new server's name is already set for another server, it will be called webserv_42_[default_name_index] instead
				for (std::map<std::string, Server*>::iterator it = server_list.begin(); it != server_list.end(); it++)
				{
					if (it->second->getServerName() == result)
					{
						default_name.append(std::to_string(default_name_index++));
	               		return default_name;
					}
				}
				return (result);
			}
		}
		default_name.append(std::to_string(default_name_index++));
		return default_name;
	}

	std::string getServerBlock(std::ifstream &ifs)
	{
		int brackets = 1;
		std::string buffer, server_block = "";

		while (brackets)
		{
			getline(ifs, buffer);
			buffer = trim(buffer);
			server_block.append(buffer);
			server_block.append("\n");
			if (buffer.find('{') != buffer.npos)
				brackets++;
			else if (buffer.find('}') != buffer.npos)
				brackets--;
		}
		server_block.pop_back();
		server_block.pop_back();
		while (server_block.back() == ' ' || server_block.back() == '\t' || server_block.back() == '\n')
			server_block.pop_back();
		return server_block;
	}

};