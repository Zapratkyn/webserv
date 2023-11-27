#ifndef __SERVER_HPP__
# define __SERVER_HPP__

#include "utils/server_utils.hpp"
#include "utils/utils.hpp"

class Server {

private:

	std::vector<std::string>									_server_name;
	std::string													_root;
	std::string													_index;
	int															_client_max_body_size;
	std::map<std::string, t_location>							_location_list;
	std::vector<struct sockaddr_in>								_end_points;

	bool 										parseOption(const int &, std::string &, std::stringstream &, const std::string &, std::vector<int> &);
	void										addDefaultLocation();

public:

	Server();
	~Server();

	std::vector<std::string> 							getServerName() const;
	std::string 										getRoot() const;
	std::string 										getIndex() const;
	int													getBodySize() const;
	std::map<std::string, t_location> 					getLocationlist() const;
	std::vector<std::pair<std::string, std::string> >	getEndPoints() const;

	bool										setHost(const std::string&);
	bool										addServerName(const std::string&);
	bool										setRoot(std::string&);
	bool										setIndex(const std::string&);
	bool										setBodySize(const std::string&);
	bool										addEndPoint(const std::string&, std::vector<int>&);
	bool										addLocation(std::stringstream&, std::string &value);

	bool										parseServer(const std::string &, const std::string &, std::vector<int> &, std::vector<std::string> &, int);
	void										handleRequest(struct t_request &, std::vector<std::string> &, bool &);

};

#endif
