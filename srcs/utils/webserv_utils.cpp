#include "../../include/utils/webserv_utils.hpp"

namespace webserv_utils {
	
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
		socketAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	}

	// void initTimeval(struct timeval &tv)
	// {
	// 	tv.tv_sec = 5;
	// 	tv.tv_usec = 0;
	// }

	// Namespaces allow us to use the same function name in different contexts
	void ft_error(int type)
	{
		switch (type) {
			case 0:
				std::cerr << "Select error" << std::endl;
				break;
			case 1:
				std::cerr << "Server failed to accept incoming connection from ADDRESS: ";
				break;
		}
	}

	void listenLog(struct sockaddr_in &socketAddr, std::map<std::string, Server*> &server_list)
	{
		std::ostringstream 	ss;
		std::vector<int>	port_list;

	   	ss << "\n\n### Webserv started ###\n\n"
		<< "\n***\n\nListening on ADDRESS: " 
	    << inet_ntoa(socketAddr.sin_addr) // inet_ntoa converts the Internet Host address to an IPv4 address (xxx.xxx.xxx.xxx)
	    << " (localhost)\n\nLISTENING PORTS:\n\n";
		for (std::map<std::string, Server*>::iterator server_it = server_list.begin(); server_it != server_list.end(); server_it++)
		{
			port_list = server_it->second->getPorts();
			for (std::vector<int>::iterator port_it = port_list.begin(); port_it != port_list.end(); port_it++)
				ss << " - " << *port_it << "\n";
		}
	    ss << "\n***\n\n";
		std::cout << ss.str() << std::endl;
	}

	std::string getServer(std::map<std::string, Server*> &server_list, int &socket)
	{
		std::vector<int> socket_list;
		std::string result = "";

		for (std::map<std::string, Server*>::iterator server_it = server_list.begin(); server_it != server_list.end(); server_it++)
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

	void parseUrl(std::string folder, std::vector<std::string> &url_list)
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
	        if (extension != ".html" && extension != ".htm" && extension != ".php")
	        {
	            sub_folder = folder_cpy.append(file_name);
				folder_cpy = folder;
				sub_folder.append("/");
	            parseUrl(sub_folder, url_list);
	        }
			else
			{
				file_name = folder_cpy.append(file_name);
				folder_cpy = folder;
				if (file_name != "./www/hello.html")
					url_list.push_back(file_name);
			}
	        file = readdir(dir);
		}
		closedir(dir);
	}

	void getRequest(int socket, int max_body_size, std::string &request_header, std::string &request_body)
	{
		int bytesReceived;
		char buffer[100000] = {0};

		bytesReceived = read(socket, buffer, 100000);
		if (bytesReceived < 0)
			throw readRequestException();

		std::string oBuffer(buffer);
		std::stringstream ifs(oBuffer);

		while (!ifs.eof() && oBuffer.size())
		{
			getline(ifs, oBuffer);
			if (oBuffer.size() != 0)
			{
				request_header.append(oBuffer);
				request_header.append("\n");
			}
		}
		while (!ifs.eof())
		{
			getline(ifs, oBuffer);
			request_body.append(oBuffer);
			request_body.append("\n");
		}
		if (request_body.size() > (size_t)max_body_size)
			throw requestBodyTooBigException();
		if (DISPLAY_REQUEST)
			std::cout << request_header << "\n" << request_body << std::endl;
	}

	void setRequest(t_request &request, std::string &request_header, std::string &request_body, std::vector<std::string> &url_list)
	{
		std::stringstream 	r_h(request_header);
		std::string			buffer, dot = ".";
		int					line = 0;

		request.url = "./www/hello.html";
		request.code = "200 OK";
		request.is_url = false;
		request.is_kill = false;

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
					errorPage(request);
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
			request.is_kill = true;

		if (request.location.substr(0, 7) == "http://")
		{
			request.location = &request.location[request.location.find_first_of(":") + 1];
			request.location = &request.location[request.location.find_first_of(":") + 1];
			request.location = &request.location[request.location.find_first_not_of(DIGITS)];
		}

		std::string extension = &request.location[request.location.find_last_of(".")];

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
			return;
		}

		// if (!allowedMethod(request.method, _location_list[request.location].methods))
		// 	throw forbiddenMethodException();

		(void)request_body;
	}

	bool validMethod(std::string &method)
	{
		if (method != "GET" && method != "DELETE" && method != "POST" && method != "HEAD" 
			&& method != "PUT" && method != "CONNECT" && method != "OPTIONS" && method != "TRACE"
			&& method != "PATCH")
			return false;
		return true;
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

	void errorPage(t_request request)
	{
		std::ifstream 	ifs(request.url.c_str());
		std::string		html = "", buffer;
		// We start our response by the http header with the right code
		std::string 	result = "HTTP/1.1 ";

		result.append(request.code);
		result.append("\nContent-Type: text/html\nContent-Length: ");

		while (!ifs.eof())
		{
			getline(ifs, buffer);
			html.append(buffer);
			html.append("\n");
		}
		result.append(ft_to_string(html.size())); // We append the size of the html page to the http response
		result.append("\n\n"); // The http response's header stops here
		result.append(html); // The http reponse body (html page)

		if (DISPLAY_HTML)
			std::cout << result << std::endl;

		write(request.socket, result.c_str(), result.size());
	}

};
