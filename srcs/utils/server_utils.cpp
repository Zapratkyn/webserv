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
						return loc;
					if (value.back() != '/')
						value.append("/");
					loc.root = value;
					break;
				case 1:
					if (loc.index != "")
						return loc;
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
							return loc;
						if (!loc.methods.empty())
						{
							for (std::vector<std::string>::iterator it = loc.methods.begin(); it != loc.methods.end(); it++)
							{
								if (*it == method)
									return loc;
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
					return loc;
			}
		}
		loc.valid = true;
		return loc;
	}

};