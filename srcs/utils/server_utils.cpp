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

t_location newLocation(const std::string &location_name, const std::string &location_block, std::string &root)
{
	DIR *dir;
	t_location loc;
	int option, pos;
	std::stringstream ifs(location_block);
	std::string method, buffer, name, value, slash = "/",
	                                         option_list[4] = {"root", "index", "allow_methods", "autoindex"};

	loc.location = location_name;
	loc.root = "";
	loc.index = "";
	loc.autoindex = "off";
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
			dir = opendir(value.c_str());
			if (!dir)
			{
				ft_error(9, root, "root");
				return loc;
			}
			if (value[value.size() - 1] != '/')
				value.append("/");
			closedir(dir);
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
				if (method != "GET" && method != "DELETE" && method != "POST" && method != "HEAD" && method != "PUT" &&
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
	loc.valid = true;
	return loc;
}

// Namespaces allow us to use the same function name in different contexts
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

void setRequest(t_request &request, bool &kill, std::string root)
{
	std::string first_line = request.header.substr(0, request.header.find_first_of("\n")), extension;

	request.method = first_line.substr(0, first_line.find_first_of(" \t"));
	if (!validMethod(request.method))
	{
		request.url = root.append("errors/400.html");
		request.code = "400 Bad Request";
		if (!sendText(request))
			sendError(400, request.socket);
		throw invalidMethodException();
	}
	first_line = &first_line[first_line.find_first_of(" \t")];
	first_line = &first_line[first_line.find_first_not_of(" \t")];
	request.location = first_line.substr(0, first_line.find_first_of(" \t"));

	if (request.location == "/kill")
	{
		kill = true;
		request.url = "www/kill.html";
		sendText(request);
	}

	extension = &request.location[request.location.find_last_of(".")];
	if (extension != ".css" && extension != ".ico")
		log("", "", request.location, 2);

	if (DISPLAY_METHOD_AND_LOCATION)
	{
		std::cout << "Method = " << request.method << std::endl;
		std::cout << "Location = " << request.location << std::endl;
	}
}

void checkUrl(struct t_request &request, std::string root, std::string &autoindex)
{
	std::ifstream file;
	std::string extension, folder;
	size_t pos;

	request.url = root;
	request.url.append(&request.location[1]);
	file.open(request.url.c_str());
	if (!file.fail())
	{
		file.close();
		pos = request.location.rfind('.');
		if (pos == std::string::npos)
		{
			if (autoindex == "on")
			{
				if (folder[folder.size() - 1] != '/')
					folder = request.url.append("/");
				request.url = root;
				request.url.append("pages/dir.html");
				sendTable(request, root, folder);
				return;
			}
		}
		else
		{
			extension = &request.location[request.location.rfind('.')];
			if (extension == ".html" || extension == ".htm" || extension == ".php")
				sendText(request);
			else
				sendFile(request);
			return;
		}
	}
	request.url = root;
	request.url.append("errors/404.html");
	request.code = "404 Not found";
	if (!sendText(request))
		sendError(404, request.socket);
}

bool checkLocation(struct t_request &request, std::map<std::string, struct t_location> &location_list, std::string root)
{
	std::ifstream index;

	for (std::map<std::string, struct t_location>::iterator it = location_list.begin(); it != location_list.end(); it++)
	{
		if (request.location == it->first)
		{
			if (it->second.index != "")
			{
				request.url = it->second.root;
				request.url.append(it->second.index);
				index.open(request.url.c_str());
				if (!index.fail())
				{
					sendText(request);
					index.close();
				}
				else
				{
					request.code = "404 Not found";
					request.url = root.append("errors/404.html");
					if (!sendText(request))
						sendError(404, request.socket);
				}
			}
			else if (it->second.autoindex == "on")
			{
				request.url = root;
				request.url.append("pages/dir.html");
				sendTable(request, root, it->second.root);
			}
			return true;
		}
	}
	return false;
}

void sendTable(struct t_request &request, std::string root, std::string folder)
{
	std::ifstream ifs(request.url.c_str());
	std::string html = "", buffer;
	std::string result = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";

	while (!ifs.eof())
	{
		getline(ifs, buffer);
		html.append(buffer);
		html.append("\n");
	}
	ifs.close();

	html.insert(html.find("</caption>"), request.location);

	if (isChildDirectory(folder, root))
		addParentDirectory(html, folder);
	
	addLinkList(html, folder);

	result.append(ft_to_string(html.size())); // We append the size of the html page to the http response
	result.append("\n\n");                    // The http response's header stops here
	result.append(html);                      // The http reponse body (html page)
	
	write(request.socket, result.c_str(), result.size());
}

void addParentDirectory(std::string &html, std::string folder)
{
	int spot = html.rfind("</tbody>");

	if (folder[folder.size() - 1] == '/')
		folder = folder.substr(0, folder.rfind('/'));
	folder = folder.substr(0, folder.rfind('/'));
	folder = folder.substr(0, folder.rfind('/'));
	
	html.insert(spot, "\n\t\t<tr>\n\t\t\t<td><img src=\"/assets/parentDirectory.png\"></td>\n\t\t\t<td><a href=\"");
	spot = html.rfind("</tbody>");
	html.insert(spot, folder);
	spot = html.rfind("</tbody>");
	html.insert(spot, "\">Parent directory</a></td>\n\t\t\t<td>Directory</td>\n\t\t</tr>\n");
}

void addLinkList(std::string &html, std::string location)
{
	std::string file_name, extension, slash = "/";
	DIR *dir = opendir(location.c_str());
	struct dirent *file = readdir(dir);
	int spot;

	while (file)
	{
		file_name = file->d_name;
		if (file_name == "." || file_name == "..")
		{
			file = readdir(dir);
			continue;
		}
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
		html.insert(spot, file_name);
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

}; // namespace server_utils
