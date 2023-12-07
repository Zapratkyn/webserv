#include "../../include/utils/webserv_utils.hpp"

namespace webserv_utils
{

bool checkRedirectionList(std::vector<std::string> &url_list)
{
	std::ifstream list("./others/redirections.list");
	std::string buffer, url, dot;
	bool is_url;

	while (!list.eof())
	{
		is_url = false;
		dot = ".";
		getline(list, buffer);
		url = dot.append(buffer.substr(buffer.find_first_of(":") + 1));
		for (std::vector<std::string>::iterator it = url_list.begin(); it != url_list.end(); it++)
		{
			if (url == *it)
			{
				is_url = true;
				break;
			}
		}
		if (!is_url)
		{
			ft_error(2, url);
			return false;
		}
	}
	return true;
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
	while (server_block[server_block.size() - 1] == ' ' || server_block[server_block.size() - 1] == '\t' ||
	       server_block[server_block.size() - 1] == '\n')
		server_block = ft_pop_back(server_block);
	return server_block;
}

// Namespaces allow us to use the same function name in different contexts
void ft_error(int type, std::string value)
{
	switch (type)
	{
	case 0:
		std::cerr << "Select error" << std::endl;
		break;
	case 1:
		std::cerr << "Server failed to accept incoming connection from ADDRESS: ";
		break;
	case 2:
		std::cerr << "ERROR\nRedirection: " << &value[1] << ": no matching file" << std::endl;
	}
}

bool socketIsSet(std::map<int, struct sockaddr_in> &socket_list, struct sockaddr_in &addr)
{
	for (std::map<int, struct sockaddr_in>::iterator it = socket_list.begin(); it != socket_list.end(); it++)
	{
		if (it->second.sin_port == addr.sin_port && it->second.sin_addr.s_addr == addr.sin_addr.s_addr)
			return true;
	}
	return false;
}

void getPotentialServers(std::vector<Server *> &server_list, struct sockaddr_in &addr, struct t_request &request)
{
	std::vector<struct sockaddr_in> end_points;

	for (std::vector<Server *>::iterator server_it = server_list.begin(); server_it != server_list.end(); server_it++)
	{
		end_points = (*server_it)->getEndPoints();
		for (std::vector<struct sockaddr_in>::iterator end_point_it = end_points.begin();
		     end_point_it != end_points.end(); end_point_it++)
		{
			if (end_point_it->sin_addr.s_addr == addr.sin_addr.s_addr && end_point_it->sin_port == addr.sin_port)
				request.potentialServers.push_back(*server_it);
		}
	}
}

void getServer(struct t_request &request)
{
	std::vector<std::string> names;
	Server *server = *request.potentialServers.begin();

	for (std::vector<Server *>::iterator server_it = request.potentialServers.begin();
	     server_it != request.potentialServers.end(); server_it++)
	{
		names = (*server_it)->getServerNames();
		for (std::vector<std::string>::iterator name_it = names.begin(); name_it != names.end(); name_it++)
		{
			if (*name_it == request.host)
				request.server = *server_it;
		}
	}
	if (!request.server)
		request.server = server;
}

void printSocketAddress(struct sockaddr_in &_socketAddr)
{
	char s[INET_ADDRSTRLEN] = {};

	inet_ntop(AF_INET, (void *)&_socketAddr.sin_addr, s, INET_ADDRSTRLEN);
	std::cout << s << ":" << ntohs(_socketAddr.sin_port);
}

void displayServers(std::vector<Server *> &server_list)
{
	std::string value;
	int iValue;
	std::vector<int> port_list;
	std::map<std::string, t_location> location_list;
	std::vector<std::string> method_list;

	std::cout << std::endl;

	for (std::vector<Server *>::iterator it = server_list.begin(); it != server_list.end(); it++)
	{
		static int i;
		std::cout << "*******   "
		          << "server #" << i++ << "   *******" << std::endl;
		value = (*it)->getIndex();
		if (value != "")
			std::cout << "Index : " << value << std::endl;
		value = (*it)->getRoot();
		if (value != "")
			std::cout << "Root : " << value << std::endl;
		iValue = (*it)->getBodySize();
		if (iValue >= 0)
			std::cout << "Client max body size : " << iValue << std::endl;
		location_list = (*it)->getLocationlist();
		if (!location_list.empty())
		{
			std::cout << "Locations :\n";
			for (std::map<std::string, t_location>::iterator it = location_list.begin(); it != location_list.end();
			     it++)
			{
				std::cout << "  - " << it->second.location << " :\n";
				value = it->second.root;
				if (value != "")
					std::cout << "    - Root : " << value << std::endl;
				value = it->second.index;
				if (value != "")
					std::cout << "    - Index : " << value << std::endl;
				std::cout << "    - Autoindex : " << it->second.autoindex << std::endl;
				method_list = it->second.methods;
				if (!method_list.empty())
				{
					std::cout << "    - Allowed methods :\n";
					for (std::vector<std::string>::iterator it = method_list.begin(); it != method_list.end(); it++)
						std::cout << "       - " << *it << std::endl;
				}
			}
		}
		std::cout << "Listening on :\n";
		std::vector<struct sockaddr_in> endpoints = (*it)->getEndPoints();
		for (std::vector<struct sockaddr_in>::iterator it = endpoints.begin(); it != endpoints.end(); ++it)
		{
			std::cout << "    - ";
			printSocketAddress(*it);
			std::cout << std::endl;
		}
		std::cout << std::endl;
		std::cout << "*****************************" << std::endl;
	}
}

void parseUrl(std::string folder, std::vector<std::string> &url_list, std::vector<std::string> &folder_list)
{
	DIR *dir = opendir(folder.c_str());
	struct dirent *file;
	std::string file_name, extension, sub_folder, folder_cpy = folder;
	size_t pos;

	file = readdir(dir);

	while (file)
	{
		file_name = file->d_name;
		if (file_name == "." || file_name == "..")
		{
			file = readdir(dir);
			continue;
		}
		pos = file_name.find_last_of(".");
		if (pos != std::string::npos)
			extension = &file_name[pos];
		else
			extension.clear();
		if (extension == "")
		{
			sub_folder = folder_cpy.append(file_name);
			folder_cpy = folder;
			sub_folder.append("/");
			folder_list.push_back(&sub_folder[1]);
			parseUrl(sub_folder, url_list, folder_list);
		}
		else
		{
			file_name = folder_cpy.append(file_name);
			folder_cpy = folder;
			url_list.push_back(file_name);
		}

		file = readdir(dir);
	}
	closedir(dir);
}

void initRequest(struct t_request &request)
{
	request.body = "";
	request.header = "";
	request.client = "";
	request.code = "200 OK";
	request.location = "";
	request.method = "";
	request.host = "";
	request.url = "";
	request.local = "";
	request.server = NULL;
}

bool getRequest(struct t_request &request)
{
	ssize_t bytesReceived;
	char buffer[BUFFER_SIZE] = {};

	bytesReceived = recv(request.socket, buffer, BUFFER_SIZE, 0);
	if (bytesReceived < 0)
		throw readRequestException();
	else if (bytesReceived == 0)
	{
		std::cout << "client closed" << std::endl;
		return false;
	}
	std::cout << "let's go" << std::endl;
	std::string oBuffer(buffer);
	std::stringstream ifs(oBuffer);

	while (!ifs.eof() && oBuffer.size())
	{
		getline(ifs, oBuffer);
		if (oBuffer.size())
		{
			request.header.append(oBuffer);
			request.header.append("\n");
		}
		if (oBuffer.substr(0, oBuffer.find_first_of(" \t")) == "Host:")
		{
			oBuffer = &oBuffer[oBuffer.find_first_of(" \t")];
			request.host = &oBuffer[oBuffer.find_first_not_of(" \t")];
		}
		else if (oBuffer.substr(0, oBuffer.find_first_of(' ')) == "Connection:")
		{
			oBuffer = &oBuffer[oBuffer.find_first_of(' ')];
			std::stringstream ss(oBuffer);
			oBuffer.clear();
			ss >> oBuffer;
			request.headers["Connection"].push_back(oBuffer);
		}
	}
	while (!ifs.eof())
	{
		getline(ifs, oBuffer);
		request.body.append(oBuffer);
		request.body.append("\n");
	}
	if (DISPLAY_REQUEST)
	{
		std::cout << request.header << std::endl;
		if (request.body != "")
			std::cout << request.body << std::endl;
	}
	return true;
}

void deleteRequest(int socket, std::vector<struct t_request> &request_list)
{
	for (std::vector<struct t_request>::iterator it = request_list.begin(); it != request_list.end(); it++)
	{
		if (it->socket == socket)
			request_list.erase(it);
	}
}

}; // namespace webserv_utils
