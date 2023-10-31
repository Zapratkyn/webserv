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

	bool allowedMethod(std::string &method, std::vector<std::string> &list)
	{
		for (std::vector<std::string>::iterator it = list.begin(); it != list.end(); it++)
		{
			if (*it == method)
				return true;
		}
		return false;
	}

	bool validMethod(std::string &method)
	{
		if (method != "GET" && method != "DELETE" && method != "POST" && method != "HEAD" 
			&& method != "PUT" && method != "CONNECT" && method != "OPTIONS" && method != "TRACE"
			&& method != "PATCH")
			return false;
		return true;
	}

	void setRequest(t_request &request, bool &kill)
	{
		std::stringstream 	r_h(request.header);
		std::string			buffer, dot = ".";
		int					line = 0;

		while (!r_h.eof())
		{
			getline(r_h, buffer);
			line++;
			if (line == 1)
			{
				// The method is always at the start of the request, for all the browsers I tested
				request.method = buffer.substr(0, buffer.find_first_of(" \t"));
				if (!validMethod(request.method))
				{
					request.url = "./www/errors/400.html";
					request.code = "400 Bad Request";
					sendUrl(request);
					throw invalidMethodException();
				}
			}
			if (line == 1 || buffer.substr(0, buffer.find_first_of(" \t")) == "Referer:")
			{
				buffer = &buffer[buffer.find_first_of(" \t")];
				buffer = &buffer[buffer.find_first_not_of(" \t")];
				// If the location is in the first line, right after the method
				if (line == 1 && buffer.substr(0, buffer.find_first_of(" \t")) != "/favicon.ico")
					request.location = buffer.substr(0, buffer.find_first_of(" \t"));
				// If the location is on the line starting with "Referer:"
				else
					request.location = buffer.substr(0);
			}
		}

		if (DISPLAY_METHOD_AND_LOCATION)
		{
			std::cout << "Method = " << request.method << std::endl;
			std::cout << "Location = " << request.location << std::endl;
		}

		if (request.location == "/kill")
		{
			kill = true;
			request.url = "./www/kill.html";
		}

		// if (!allowedMethod(request.method, _location_list[request.location].methods))
		// 	throw forbiddenMethodException();

		(void)request.body;
	}

	void checkUrl(struct t_request &request, std::vector<std::string> &url_list)
	{
		std::string dot = ".";

		if (request.location.substr(0, 7) == "http://")
		{
			request.location = &request.location[request.location.find_first_of(":") + 1];
			request.location = &request.location[request.location.find_first_of(":") + 1];
			request.location = &request.location[request.location.find_first_not_of(DIGITS)];
		}

		std::string extension = &request.location[request.location.find_last_of(".")];

		log("", request.client, request.server, request.location, 2);

		if (extension == ".html" || extension == ".htm" || extension == ".php")
		{
			request.is_url = true;
			request.location = dot.append(request.location);
			for (std::vector<std::string>::iterator it = url_list.begin(); it != url_list.end(); it++)
			{
				/*
				If the requested url exists in the Webserv's list, we provide the page
				If not, we provide the 404 error page (default request.url at the start of the function)
				*/
				if (*it == request.location)
				{
					request.url = *it;
					return;
				}
			}
			request.url = "./www/errors/404.html";
			request.code = "404 Not found";
		}
	}

	void checkLocation(struct t_request &request, std::map<std::string, struct t_location> &location_list)
	{
		(void)request;
		(void)location_list;
	}

};
