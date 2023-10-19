#include "../include/Webserv.hpp"

using namespace webserv_utils;
// using namespace server_utils;

Webserv::Webserv(std::string conf_file) : _conf(conf_file)
{
	return;
}
Webserv::~Webserv() 
{
	for (std::map<std::string, Server*>::iterator it = _server_list.begin(); it != _server_list.end(); it++)
		delete it->second;
	return;
}

// t_location new_location(std::string &location_name, std::string &location_block)
// {
// 	t_location loc;
// 	int option, pos;
// 	std::ifstream ifs(location_block);
// 	std::string method, buffer, name, value, option_list[3] = {"root", "index", "allow_methods"};

// 	loc.location = location_name;
// 	loc.root = "";
// 	loc.index == "";
// 	loc.valid = false;

// 	while (!ifs.eof())
// 	{
// 		getline(ifs, buffer);
// 		name = getOptionName(buffer);
// 		value = getOptionValue(buffer);
// 		for (int i = 0; i <= 3; i++)
// 		{
// 			option = i;
// 			if (name == option_list[i])
// 				break;
// 		}
// 		switch (option) {
// 			case 0:
// 				if (loc.root != "")
// 					return loc;
// 				loc.root = value;
// 				break;
// 			case 1:
// 				if (loc.index != "")
// 					return loc;
// 				loc.index = value;
// 				break;
// 			case 2:
// 				value.push_back(' ');
// 				while (value[1])
// 				{
// 					pos = value.find_first_of(" \t");
// 					method = value.substr(0, pos);
// 					if (!loc.methods.empty())
// 					{
// 						for (std::vector<std::string>::iterator it = loc.methods.begin(); it != loc.methods.end(); it++)
// 						{
// 							if (*it == method)
// 								return loc;
// 						}
// 						loc.methods.push_back(method);
// 					}
// 					value = &value[pos];
// 				}
// 				break;
// 			default:
// 				return loc;
// 		}
// 	}
// 	loc.valid = true;
// 	return loc;
// }

// bool parseOption(Server *server, int option, std::string &value, std::ifstream &ifs)
// {
// 	std::string buffer = "", tmp;
// 	std::vector<int> port_list = server->getPorts();
// 	std::map<std::string, t_location> location_list = server->getLocationlist();
// 	int iValue;

// 	switch (option) {
// 		case 0:
// 			if (value.find_first_not_of(DIGITS) != value.npos)
// 				return false;
// 			iValue = std::stoi(value);
// 			if (!port_list.empty())
// 			{
// 				for (std::vector<int>::iterator it = port_list.begin(); it != port_list.end(); it++)
// 				{
// 					if (*it == iValue)
// 						return false;
// 				}
// 			}
// 			server->getPorts().push_back(iValue);
// 			break;
// 		case 1:
// 			if (server->getHost() != "")
// 				return false;
// 			server->setHost(value);
// 			break;
// 		case 2:
// 			if (server->getServerName() != "")
// 				return false;
// 			server->setServerName(value);
// 			break;
// 		case 3:
// 			if (server->getBodySize())
// 				return false;
// 			server->setBodySize(std::stoi(value));
// 			break;
// 		case 4:
// 			if (server->getRoot() != "")
// 				return false;
// 			server->setRoot(value);
// 			break;
// 		case 5:
// 			if (server->getIndex() != "")
// 				return false;
// 			server->setIndex(value);
// 			break;
// 		case 6:
// 			if (!location_list.empty())
// 			{
// 				if (location_list.find(value) != location_list.end())
// 					return false;
// 			}
// 			while (tmp.back() != '}')
// 			{
// 				getline(ifs, buffer);
// 				buffer = trim(buffer);
// 				tmp.append(buffer);
// 				if (tmp.back() != '}')
// 					tmp.append("\n");
// 			}
// 			tmp.pop_back();
// 			server->getLocationlist()[value] = new_location(value, tmp);
// 			if (!server->getLocationlist()[value].valid)
// 			{
// 				delete server;
// 				return NULL;
// 			}
// 			break;
// 	}
// 	return true;
// }

// bool parseServer(Server *server, std::string server_block)
// {
// 	std::string 	buffer, name, value, option_list[7] = {"listen", "host", "server_name", "client_max_body_size", "root", "index", "location"};
// 	std::ifstream	ifs(server_block);
// 	int				pos, option;

// 	while (!ifs.eof())
// 	{
// 		getline(ifs, buffer);
// 		buffer = trim(buffer);
// 		if (!buffer.size())
// 			continue;
// 		if (buffer.back() != ';' && buffer.back() != '{')
// 		{
// 			delete server;
// 			return NULL;
// 		}
// 		name = getOptionName(buffer);
// 		value = getOptionValue(buffer);
// 		for (int i = 0; i <= 7; i++)
// 		{
// 			option = i;
// 			if (name == option_list[i])
// 				break;
// 		}
// 		if (option == 8 || !parseOption(server, option, value, ifs))
// 		{
// 			delete server;
// 			return NULL;
// 		}
// 	}
// }

void Webserv::parseConf()
{
	std::ifstream 	infile(_conf);
	std::string		buffer, server_block, server_name;
	size_t			brackets = 0, default_name = 1;
	Server			*server;

	while(!infile.eof())
	{
		getline(infile, buffer);
		if (buffer == "server {")
		{
			server_block = "";
			brackets++;
			while (brackets)
			{
				getline(infile, buffer);
				buffer = trim(buffer);
				server_block.append(buffer);
				server_block.append("\n");
				if (buffer.find('{'))
					brackets++;
				else if (buffer.find('}'))
					brackets--;
			}
			while (server_block.back() != '}')
				server_block.pop_back();
			server_block.pop_back();
			while (server_block.back() == ' ' || server_block.back() == '\t' || server_block.back() == '\n')
				server_block.pop_back();
			server = new Server;
			server_name = getServerName(server_block);
			if (server_name == "webserv_42_")
				server_name = server_name.append(std::to_string(default_name++));
			if (!server->parseServer(server_block))
				throw wrongOptionException();
			_server_list[server_name] = server;
		}
	}
}

// void Webserv::startServer()
// {
// 	/*Creating a socket, a communication entry point. 
// 	AF_INET for TCP/IP protocol. AF stands for Address Family (in this case, IPv4)
// 	SOCK_STREAM is the type of communication > Safe binary stream
// 	0 is the default protocol*/
// 	_socket = socket(AF_INET, SOCK_STREAM, 0); 
// 	if (_socket < 0)
// 		throw openSocketException();
	
// 	initAddr(); // Since the _socketAddr is a data structure, it needs to be set for incoming uses

// 	if (bind(_socket, (sockaddr *)&_socketAddr, _socketAddrLen) < 0) // Binding the socket to the address
// 		throw bindException();
// }

// void Webserv::startListen()
// {
// 	if (listen(_socket, 10) < 0) // Start listening on the socket, with a maximum of 10 connections at a time
// 		throw listenException();
	
// 	listenLog(); // Display informations about the listening socket

// 	int bytesReceived;

// 	while (true)
// 	{
// 		std::cout << "Waiting for new connection...\n\n" << std::endl;
// 		if (!newConnection(_new_socket))
// 			std::cerr << "Server failed to accept incoming connection from ADDRESS: " << 
// 			inet_ntoa(_socketAddr.sin_addr) << "; PORT: " << 
// 			ntohs(_socketAddr.sin_port) << std::endl;
// 		else
// 		{
// 			char buffer[30720] = {0};
//            	bytesReceived = read(_new_socket, buffer, 30720); // since we are using AF_INET, the requests will be streams we can read()
//             if (bytesReceived < 0)
// 				std::cerr << "Failed to read bytes from client socket connection" << std::endl;
// 			else
// 			{
// 				std::cout << "Request received from the client\n" << std::endl;
// 				sendResponse();
// 			}
// 			close(_new_socket);
// 		}
// 	}
// }

// bool Webserv::newConnection(int &new_socket)
// {
// 	new_socket = accept(_socket, (sockaddr *)&_socketAddr, &_socketAddrLen); // _socket is the listening socket, never changes. _socketAddr is teh struct used for each new connection
// 	if (new_socket < 0)
// 		return false;
// 	return true;
// }

// void Webserv::sendResponse()
// {
//     unsigned long bytesSent;

//     bytesSent = write(_new_socket, _serverMessage.c_str(), _serverMessage.size());

//     if (bytesSent == _serverMessage.size())
//         std::cout << "------ Server Response sent to client ------\n" << std::endl;
//     else
//         std::cerr << "Error sending response to client" << std::endl;
// }

// std::string Webserv::buildResponse()
// {
// 	std::string htmlFile = "<!DOCTYPE html><html lang=\"en\"><body><h1> HOME </h1><p> Hello from your Server :) </p></body></html>";
//     std::ostringstream ss;
//     ss << "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: " << htmlFile.size() << "\n\n"
//        << htmlFile;

// 	return ss.str();
// }

// void Webserv::initAddr()
// {
// 	_socketAddr.sin_family = AF_INET; // The socket's address family
// 	_socketAddr.sin_port = htons(_port); // Copies the port number. htons ensures the bytes order is respected (stands for Host to Network Short)
// 	_socketAddr.sin_addr.s_addr = htonl(INADDR_ANY); // INADDR_ANY = "0.0.0.0"
// }
// void Webserv::listenLog() const
// {
// 	std::ostringstream ss;
//    	ss << "\n*** Listening on ADDRESS: " 
//     << inet_ntoa(_socketAddr.sin_addr)  // inet_ntoa converts the Internet Host address to an IPv4 address (xxx.xxx.xxx.xxx)
//     << " PORT: " << ntohs(_socketAddr.sin_port)  // Copies the port number. ntohs ensures the bytes order is respected (stands for Network to Host Short)
//     << " ***\n\n";
// 	std::cout << ss.str() << std::endl;
// }
