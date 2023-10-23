#include "../../include/utils/webserv_utils.hpp"

namespace webserv_utils {

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
					default_name.append(ft_to_string(default_name_index++));
	                return default_name;
				}
				result = buffer.substr(0);
				while (result[result.size() - 1] == ' ' || result[result.size() - 1] == '\t' || result[result.size() - 1] == ';' || result[result.size() - 1] == '}')
					result = ft_pop_back(result);
				// If the new server's name is already set for another server, it will be called webserv_42_[default_name_index] instead
				for (std::map<std::string, Server*>::iterator it = server_list.begin(); it != server_list.end(); it++)
				{
					if (it->second->getServerName() == result)
					{
						default_name.append(ft_to_string(default_name_index++));
	               		return default_name;
					}
				}
				return (result);
			}
		}
		default_name.append(ft_to_string(default_name_index++));
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
		server_block = ft_pop_back(server_block);
		server_block = ft_pop_back(server_block);
		while (server_block[server_block.size() - 1] == ' ' || server_block[server_block.size() - 1] == '\t' || server_block[server_block.size() - 1] == '\n')
			server_block = ft_pop_back(server_block);
		return server_block;
	}

	void initSockaddr(struct sockaddr_in &socketAddr)
	{
		socketAddr.sin_family = AF_INET;
		socketAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}

};