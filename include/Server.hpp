#ifndef __SERVER_HPP__
# define __SERVER_HPP__

#include <iostream>
#include <netinet/in.h>
#include <vector>

struct t_location {

	std::string					location;
	std::string					root;
	std::string					index;
	std::vector<std::string>	methods;

};

class Server {

private:

	std::string						_host;
	std::string						_server_name;
	std::string						_root;
	std::string						_index;
	int								_client_max_body_size;
	std::vector<int>				_ports;
	std::vector<t_location>			_location_list;

public:

	Server();
	~Server();

	std::string	getHost() const;
	std::string getServerName() const;
	std::string getRoot() const;
	std::string getIndex() const;
	int			getBodySize() const;
	std::vector<int> getPorts() const;
	std::vector<t_location> getLocationlist() const;

	void	setHost(std::string);
	void	setServerName(std::string);
	void	setRoot(std::string);
	void	setIndex(std::string);
	void	setBodySize(int);

};

#endif