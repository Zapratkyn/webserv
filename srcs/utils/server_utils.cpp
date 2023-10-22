#include "../../include/utils/server_utils.hpp"

namespace server_utils {

	std::string getOptionName(const std::string &str) 
	{ 
		std::string result = str.substr(0, str.find_first_of(" \t"));

		return result;
	}

	std::string getOptionValue(const std::string &str)
	{
	    std::string result = &str[str.find_first_of(" \t")];

	    result = &result[result.find_first_not_of(" \t")];

	    while (result.back() == '{' || result.back() == ';' || result.back() == ' ' || result.back() == '\t')
	        result.pop_back();

	    return result;
	}

	std::string getLocationBlock(std::stringstream &ifs)
	{
		std::string location_block, buffer;

		while (location_block.back() != '}')
		{
			getline(ifs, buffer);
			if (buffer[0] == '{')
				continue;
			location_block.append(buffer);
			if (location_block.back() != '}')
				location_block.append("\n");
		}
		location_block.pop_back();
		if (location_block.back() == '\n')
			location_block.pop_back();
		return location_block;
	}

	t_location newLocation(const std::string &location_name, const std::string &location_block)
	{
		t_location loc;
		int option, pos;
		std::stringstream ifs(location_block);
		std::string method, buffer, name, value, option_list[3] = {"root", "index", "allow_methods"};

		loc.location = location_name;
		loc.root = "";
		loc.index = "";
		loc.valid = false;

		while (!ifs.eof())
		{
			getline(ifs, buffer);
			if (buffer.back() != ';')
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
					if (value.back() != '/')
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

};