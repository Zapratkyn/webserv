#include "../../include/utils/server_utils.hpp"

namespace server_utils
{

std::string getOptionName(const std::string &str)
{
	std::string result = str.substr(0, str.find_first_of(" \t"));

	return result;
}

std::string getOptionValue(const std::string &str)
{
	std::string result = &str[str.find_first_of(" \t")];

	result = &result[result.find_first_not_of(" \t")];

	while (result[result.size() - 1] == '{' || result[result.size() - 1] == ';' || result[result.size() - 1] == ' ' ||
	       result[result.size() - 1] == '\t')
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

t_location newLocation(const std::string &location_name, const std::string &location_block, std::string &root, std::string autoindex)
{
	t_location loc;
	int option, pos;
	std::stringstream ifs(location_block);
	std::string method, buffer, name, value, slash = "/",
	                                         option_list[4] = {"root", "index", "allow_methods", "autoindex"};

	bool allow_methods_is_not_defined = true;
	loc.location = location_name;
	loc.root = "";
	loc.index = "";
	loc.autoindex = autoindex;
	loc.valid = false;

	while (!ifs.eof())
	{
		buffer.clear();
		getline(ifs, buffer);
		if (buffer[buffer.size() - 1] != ';')
		{
			ft_error(1, buffer, buffer);
			return loc;
		}
		name = getOptionName(buffer);
		value = getOptionValue(buffer);
		for (int i = 0; i <= 4; i++)
		{
			option = i;
			if (name == option_list[i])
				break;
		}
		switch (option)
		{
		case 0:
			if (loc.root != "")
			{
				ft_error(0, value, "loc.root");
				return loc;
			}
//			if (value[value.size() - 1] != '/')
//				value.append("/");
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
			allow_methods_is_not_defined = false;
			value.push_back(' ');
			while (1)
			{
				pos = value.find_first_of(" \t");
				method = value.substr(0, pos);
				if (!method.empty() && method != "GET" && method != "DELETE" && method != "POST" && method != "HEAD" && method != "PUT" &&
				    method != "CONNECT" && method != "OPTIONS" && method != "TRACE" && method != "PATCH")
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
				if (!method.empty())
					loc.methods.push_back(method);
				value = &value[pos + 1];
				if (!value.size())
					break;
				value = &value[value.find_first_not_of(" \t")];
			}
			break;
		case 3:
			if (value != "on" && value != "off")
			{
				ft_error(5, value, "");
				return loc;
			}
			loc.autoindex = value;
			break;
		default:
			ft_error(4, name, "");
			return loc;
		}
	}
	if (loc.root == "")
		loc.root = root;
	//TODO does the server block also need an allow_methods directive?
	if (loc.methods.empty() && allow_methods_is_not_defined)
		loc.methods = Webserv::implementedMethods;
	loc.valid = true;
	return loc;
}

void ft_error(int type, std::string value, std::string option)
{
	std::cerr << "ERROR\n";
	switch (type)
	{
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
	case 5:
		std::cerr << "autoindex " << value << ": invalid value" << std::endl;
		break;
	case 6:
		std::cerr << "Listen directive is empty" << std::endl;
		break;
	case 7:
		std::cerr << "listen: invalid value" << std::endl;
		break;
	case 8:
		std::cerr << option << " " << value << ": missing url" << std::endl;
		break;
	case 9:
		std::cerr << value << ": no such directory" << std::endl;
		break;
	}
}

bool setSocketAddress(const std::string &ip_address, const std::string &port_num, struct sockaddr_in *socket_addr)
{
	struct addrinfo hints = {};
	struct addrinfo *res = NULL;
	int status;

	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	status = getaddrinfo(ip_address.c_str(), port_num.c_str(), &hints, &res);
	if (status != 0 || !res)
		return false;
	*socket_addr = *(struct sockaddr_in *)res->ai_addr;
	freeaddrinfo(res);
	return true;
}

int getSocketAddress(int socket, struct sockaddr_in *addr)
{
	socklen_t len = sizeof *addr;
	return (getsockname(socket, (struct sockaddr *)addr, &len));
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
	if (method != "GET" && method != "DELETE" && method != "POST" && method != "HEAD" && method != "PUT" &&
	    method != "CONNECT" && method != "OPTIONS" && method != "TRACE" && method != "PATCH")
		return false;
	return true;
}

void setResponse(t_request &request, bool &kill, std::string root)
{
	(void)root;
	std::string first_line = request.header.substr(0, request.header.find_first_of("\n")), extension;

	request.method = first_line.substr(0, first_line.find_first_of(" \t"));
	request.location = "www/server00/pages/test.html";
	request.url = request.location;
	request.code = "200 OK";
	sendText(request);

	(void)kill;

}

void addParentDirectory(std::string &html, std::string local, std::string root)
{
	int spot = html.rfind("</tbody>");

	if (local[local.size() - 1] == '/')
		local = local.substr(0, local.rfind("/"));
	local = local.substr(0, local.rfind("/") + 1);

	if (local == "/")
	{
		local = root;
		local.insert(0, "/");
	}

	html.insert(spot, "\n\t\t<tr>\n\t\t\t<td><img src=\"/assets/parentDirectory.png\"></td>\n\t\t\t<td><a href=\"");
	spot = html.rfind("</tbody>");
	html.insert(spot, local);
	spot = html.rfind("</tbody>");
	html.insert(spot, "\">Parent directory</a></td>\n\t\t\t<td>Directory</td>\n\t\t</tr>\n");
}

void addLinkList(std::string &html, std::string location, std::string local)
{
	std::string file_name, extension, slash = "/";
	DIR *dir = opendir(location.c_str());
	struct dirent *file = readdir(dir);
	int spot;

	if (local[local.size() - 1] != '/')
		local.append("/");

	while (file)
	{
		file_name = file->d_name;
		if (file_name == "." || file_name == "..")
		{
			file = readdir(dir);
			continue;
		}
		file_name.insert(0, local);
		extension = &file_name[file_name.find_last_of(".")];
		spot = html.rfind("</tbody>");
		html.insert(spot, "\t\t<tr>\n\t\t\t<td><img src=\"");
		spot = html.rfind("</tbody>");
		if (extension == ".html" || extension == ".htm" || extension == ".php")
			html.insert(spot, "/assets/webPage.png");
		else if (extension[0] == '.')
			html.insert(spot, "/assets/file.png");
		else
			html.insert(spot, "/assets/directory.png");
		spot = html.rfind("</tbody>");
		html.insert(spot, "\" /></td>\n\t\t\t<td><a href=\"");
		spot = html.rfind("</tbody>");
		html.insert(spot, file_name);
		spot = html.rfind("</tbody>");
		html.insert(spot++, 1, '"');
		html.insert(spot++, 1, '>');
		html.insert(spot, &file_name[file_name.rfind("/")] + 1);
		spot = html.rfind("</tbody>");
		html.insert(spot, "</a></td>\n\t\t\t<td>");
		spot = html.rfind("</tbody>");
		if (extension == ".html" || extension == ".htm" || extension == ".php")
			html.insert(spot, "Web page");
		else if (extension[0] == '.')
			html.insert(spot, "File");
		else
			html.insert(spot, "Directory");
		spot = html.rfind("</tbody>");
		html.insert(spot, "</td>\n\t\t</tr>\n\t");
		file = readdir(dir);
	}
	closedir(dir);
}

bool displayRoot(struct t_request &request, std::string root, std::string autoindex)
{
	if (&request.location[1] != root || autoindex == "off")
		return false;

	DIR *dir = opendir(root.c_str());
	std::ifstream ifs(root.append("assets/dir.html"));
	struct dirent *file = readdir(dir);
	int spot;

	std::string html = "", buffer, file_name, extension;
	std::string result = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";

	while (!ifs.eof())
	{
		getline(ifs, buffer);
		html.append(buffer);
		html.append("\n");
	}
	ifs.close();

	html.insert(html.find("</caption>"), request.location);

	while (file)
	{
		file_name = file->d_name;
		if (file_name == "." || file_name == "..")
		{
			file = readdir(dir);
			continue;
		}
		file_name.insert(0, "/");
		extension = &file_name[file_name.find_last_of(".")];
		spot = html.rfind("</tbody>");
		html.insert(spot, "\t\t<tr>\n\t\t\t<td><img src=\"");
		spot = html.rfind("</tbody>");
		if (extension == ".html" || extension == ".htm" || extension == ".php")
			html.insert(spot, "/assets/webPage.png");
		else if (extension[0] == '.')
			html.insert(spot, "/assets/file.png");
		else
			html.insert(spot, "/assets/directory.png");
		spot = html.rfind("</tbody>");
		html.insert(spot, "\" /></td>\n\t\t\t<td><a href=\"");
		spot = html.rfind("</tbody>");
		html.insert(spot, file_name);
		spot = html.rfind("</tbody>");
		html.insert(spot++, 1, '"');
		html.insert(spot++, 1, '>');
		html.insert(spot, &file_name[1]);
		spot = html.rfind("</tbody>");
		html.insert(spot, "</a></td>\n\t\t\t<td>");
		spot = html.rfind("</tbody>");
		if (extension == ".html" || extension == ".htm" || extension == ".php")
			html.insert(spot, "Web page");
		else if (extension[0] == '.')
			html.insert(spot, "File");
		else
			html.insert(spot, "Directory");
		spot = html.rfind("</tbody>");
		html.insert(spot, "</td>\n\t\t</tr>\n\t");
		file = readdir(dir);
	}
	closedir(dir);

	result.append(ft_to_string(html.size())); // We append the size of the html page to the http response
	result.append("\n\n");                    // The http response's header stops here
	result.append(html);                      // The http reponse body (html page)

	write(request.socket, result.c_str(), result.size());

	return true;
}

}; // namespace server_utils
