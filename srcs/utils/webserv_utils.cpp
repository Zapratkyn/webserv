#include "../../include/utils/webserv_utils.hpp"

namespace webserv_utils {

	/*
	Because of the -std=c++98 flag, we can't use pop_back() and std::to_string()
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
		socketAddr.sin_addr.s_addr = inet_addr("127.0.0.1"); // Allows any address to reach the server
	}

	// void initTimeval(struct timeval &tv)
	// {
	// 	tv.tv_sec = 5;
	// 	tv.tv_usec = 0;
	// }

	// Namespaces allow us to use the same function name in different contexts
	void ft_error(int type, struct sockaddr_in sockaddr)
	{
		switch (type) {
			case 0:
				std::cerr << "Select error" << std::endl;
				break;
			case 1:
				std::cerr << "Server failed to accept incoming connection from ADDRESS: " << 
				inet_ntoa(sockaddr.sin_addr) << std::endl;
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
	    << " (localhost)\n\nPORTS:\n\n";
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
				{
					result = server_it->second->getServerName();
					break;
				}
			}
			if (result != "")
				break;
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

	void displayRequest(int socket)
	{
		int bytesReceived;
		char buffer[640001] = {0};

		bytesReceived = read(socket, buffer, 64001);
		if (bytesReceived > 64000)
			std::cerr << "Client's body size to big" << std::endl;
		else if (bytesReceived < 0)
			std::cerr << "Error while reading request";
		write(1, buffer, bytesReceived);
	}

};