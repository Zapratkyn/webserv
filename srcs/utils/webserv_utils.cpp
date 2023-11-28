#include "../../include/utils/webserv_utils.hpp"

namespace webserv_utils {
	
	bool checkRedirectionList(std::vector<std::string> &url_list)
	{
		std::ifstream	list("./redirections.list");
		std::string		buffer, url, dot;
		bool			is_url;

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
		while (server_block[server_block.size() - 1] == ' ' || server_block[server_block.size() - 1] == '\t' || server_block[server_block.size() - 1] == '\n')
			server_block = ft_pop_back(server_block);
		return server_block;
	}

	// Namespaces allow us to use the same function name in different contexts
	void ft_error(int type, std::string value)
	{
		switch (type) {
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

	bool socketIsSet(std::map<int, struct sockaddr_in> &socket_list, struct sockaddr_in addr)
	{
		for (std::map<int, struct sockaddr_in>::iterator it = socket_list.begin(); it != socket_list.end(); it++)
		{
			if (it->second.sin_port == addr.sin_port && it->second.sin_addr.s_addr == addr.sin_addr.s_addr)
				return true;
		}
		return false;
	}

	void listenLog(struct sockaddr_in &socketAddr, std::map<std::string, Server*> &server_list)
	{
		std::ostringstream 	ss;
		std::vector<int>	port_list;

	   	ss << "\n\n### Webserv started ###\n\nLISTENING PORTS:\n\n";
		for (std::map<std::string, Server*>::iterator server_it = server_list.begin(); server_it != server_list.end(); server_it++)
		{
			port_list = server_it->second->getPorts();
			for (std::vector<int>::iterator port_it = port_list.begin(); port_it != port_list.end(); port_it++)
				ss << " - " << *port_it << "\n";
		}
	    ss << "\n***\n";
		std::cout << ss.str() << std::endl;
	}

	std::string getServer(std::vector<Server> &server_list, std::map<int, struct sockaddr_in> &socket_list)
	{
		std::vector<int> socket_list;
		std::string result = "";

		for (std::vector<Server>::iterator server_it = server_list.begin(); server_it != server_list.end(); server_it++)
		{
			socket_list = server_it->second->getSockets();
			for (std::vector<int>::iterator socket_it = socket_list.begin(); socket_it != socket_list.end(); socket_it++)
			{
				if (*socket_it == socket)
					return server_it->second->getServerName();
			}
		}
		return result;
	}

	void displayServers(std::map<std::string, Server*> &server_list)
	{
		std::string 						value;
		int									iValue;
		std::vector<int>					port_list;
		std::map<std::string, t_location>	location_list;
		std::vector<std::string>			method_list;

		std::cout << std::endl;

		for (std::map<std::string, Server*>::iterator it = server_list.begin(); it != server_list.end(); it++)
		{
			std::cout << "### " << it->first << " ###\n" << std::endl;
			value = it->second->getHost();
			if (value != "")
				std::cout << "Host : " << value << std::endl;
			value = it->second->getIndex();
			if (value != "")
				std::cout << "Index : " << value << std::endl;
			value = it->second->getRoot();
			if (value != "")
				std::cout << "Root : " << value << std::endl;
			iValue = it->second->getBodySize();
			if (iValue >= 0)
				std::cout << "Client max body size : " << iValue << std::endl;
			port_list = it->second->getPorts();
			if (!port_list.empty())
			{
				std::cout << "Ports :\n";
				for (std::vector<int>::iterator it = port_list.begin(); it != port_list.end(); it++)
					std::cout << "  - " << *it << std::endl;
			}
			location_list = it->second->getLocationlist();
			if (!location_list.empty())
			{
				std::cout << "Locations :\n";
				for (std::map<std::string, t_location>::iterator it = location_list.begin(); it != location_list.end(); it++)
				{
					std::cout << "  - " << it->second.location << " :\n";
					value = it->second.root;
					if (value != "")
						std::cout << "    - Root : " << value << std::endl;
					value = it->second.index;
					if (value != "")
						std::cout << "    - Index : " << value << std::endl;
					method_list = it->second.methods;
					if (!method_list.empty())
					{
						std::cout << "    - Allowed methods :\n";
						for (std::vector<std::string>::iterator it = method_list.begin(); it != method_list.end(); it++)
							std::cout << "       - " << *it << std::endl;
					}
				}
			}
			std::cout << std::endl;
		}
	}

	void parseUrl(std::string folder, std::vector<std::string> &url_list, std::vector<std::string> &folder_list)
	{
		DIR *dir = opendir(folder.c_str());
	    struct dirent *file;
	    std::string file_name, extension, sub_folder, folder_cpy = folder;
	
	    file = readdir(dir);
	
	    while (file)
	    {
			file_name = file->d_name;
	        if (file_name == "." || file_name == "..")
			{
	            file = readdir(dir);
	            continue; 
	        }
	        extension = &file_name[file_name.find_last_of(".")];
	        if (extension[0] != '.')
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
		request.server = "";
		request.is_url = false;
	}

	void getRequest(int max_body_size, struct t_request &request)
	{
		int bytesReceived;
		char buffer[BUFFER_SIZE];

		bytesReceived = read(request.socket, buffer, BUFFER_SIZE);
		if (bytesReceived < 0)
			throw readRequestException();

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
				oBuffer = &oBuffer(oBuffer.find_first_of(" \t"));
				request.host = &oBuffer(oBuffer.find_first_not_of(" \t"));
			}
		}
		while (!ifs.eof())
		{
			getline(ifs, oBuffer);
			request.body.append(oBuffer);
			request.body.append("\n");
		}
		if (request.body.size() > (size_t)max_body_size)
			throw requestBodyTooBigException();
		if (DISPLAY_REQUEST)
			std::cout << request.header << "\n" << request.body << std::endl;
	}

};
