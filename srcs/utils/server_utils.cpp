#include "../../include/utils/server_utils.hpp"

namespace server_utils {
	
	/*
	Because of the -std=c++98 flag, we can't use pop_back(), std::to_string() and std::stoi()
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
	
	std::string getOptionName(const std::string &str) 
	{ 
		std::string result = str.substr(0, str.find_first_of(" \t"));

		return result;
	}

	std::string getOptionValue(const std::string &str)
	{
	    std::string result = &str[str.find_first_of(" \t")];

	    result = &result[result.find_first_not_of(" \t")];

	    while (result[result.size() - 1] == '{' || result[result.size() - 1] == ';' || result[result.size() - 1] == ' ' || result[result.size() - 1] == '\t')
	        result = ft_pop_back(result);

	    return result;
	}

	std::string getLocationBlock(std::stringstream &ifs)
	{
		std::string location_block, buffer;

		while (location_block[location_block.size() - 1] != '}')
		{
			getline(ifs, buffer);
			if (buffer[0] == '{')
				continue;
			location_block.append(buffer);
			if (location_block[location_block.size() - 1] != '}')
				location_block.append("\n");
		}
		location_block = ft_pop_back(location_block);
		if (location_block[location_block.size() - 1] == '\n')
			location_block = ft_pop_back(location_block);
		return location_block;
	}

	t_location newLocation(const std::string &location_name, const std::string &location_block)
	{
		t_location loc;
		int option, pos;
		std::stringstream ifs(location_block);
		std::string method, buffer, name, value, slash = "/", option_list[3] = {"root", "index", "allow_methods"};

		loc.location = location_name;
		loc.root = "";
		loc.index = "";
		loc.valid = false;

		while (!ifs.eof())
		{
			getline(ifs, buffer);
			if (buffer[buffer.size() - 1] != ';')
			{
				ft_error(1, buffer, buffer);
				return loc;
			}
			name = getOptionName(buffer);
			value = getOptionValue(buffer);
			for (int i = 0; i <= 3; i++)
			{
				option = i;
				if (name == option_list[i])
					break;
			}
			switch (option) {
				case 0:
					if (loc.root != "")
					{
						ft_error(0, value, "loc.root");
						return loc;
					}
					if (value[0] != '/')
						value = slash.append(value);
					if (value[value.size() - 1] != '/')
						value.append("/");
					loc.root = value;
					break;
				case 1:
					if (loc.index != "")
					{
						ft_error(0, value, "loc.index");
						return loc;
					}
					loc.index = value;
					break;
				case 2:
					value.push_back(' ');
					while (1)
					{
						pos = value.find_first_of(" \t");
						method = value.substr(0, pos);
						if (method != "GET" && method != "DELETE" && method != "POST" && method != "HEAD" 
							&& method != "PUT" && method != "CONNECT" && method != "OPTIONS" && method != "TRACE"
							&& method != "PATCH")
							{
								ft_error(3, method, "");
								return loc;
							}
						if (!loc.methods.empty())
						{
							for (std::vector<std::string>::iterator it = loc.methods.begin(); it != loc.methods.end(); it++)
							{
								if (*it == method)
								{
									ft_error(0, value, "loc.allow_method");
									return loc;
								}
							}
						}
						loc.methods.push_back(method);
						value = &value[pos + 1];
						if (!value.size())
							break;
						value = &value[value.find_first_not_of(" \t")];
					}
					break;
				default:
					ft_error(4, name, "");
					return loc;
			}
		}
		loc.valid = true;
		return loc;
	}

	// Namespaces allow us to use the same function name in different contexts
	void	ft_error(int type, std::string value, std::string option)
	{
		std::cerr << "ERROR\n";
		switch (type) {
			case 0:
				std::cerr << option << " " << value << ": duplicate" << std::endl;
				break;
			case 1:
				std::cerr << option << " " << value << ": missing ';'" << std::endl;
				break;
			case 2:
				std::cerr << option << " " << value << ": Not a number" << std::endl;
				break;
			case 3:
				std::cerr << value << ": invalid method" << std::endl;
				break;
			case 4:
				std::cerr << value << ": invalid option" << std::endl;
				break;
		}
	}

	bool validMethod(std::string &method)
	{
		if (method != "GET" && method != "DELETE" && method != "POST" && method != "HEAD" 
			&& method != "PUT" && method != "CONNECT" && method != "OPTIONS" && method != "TRACE"
			&& method != "PATCH")
			return false;
		return true;
	}

	bool allowedMethod(std::string &method, std::vector<std::string> &list)
	{
		for (std::vector<std::string>::iterator it = list.begin(); it != list.end(); it++)
		{
			if (*it == method)
				return true;
		}
		return false;
	}

	void killMessage(int socket)
	{
		std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> WEBSERV SHUT DOWN </h1><p>Good bye !</p></body></html>";
    	std::string result = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";

		result.append(ft_to_string(htmlFile.size()));
		result.append("\n\n");
		result.append(htmlFile);

		write(socket, result.c_str(), result.size());
	}

};
