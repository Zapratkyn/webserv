#include "../../include/utils/UrlParser.hpp"

UrlParser::UrlParser(const std::string &url) : resource(url)
{
	size_t pos;

	if ((pos = resource.find('?')) != std::string::npos)
	{
		path = resource.substr(0, pos);
		query_string = resource.substr(pos + 1);
		if (!queryStringToParams(query_string, query_params))
			query_params.clear();
	}
	else
		path = resource;

	directory = path.substr(0, path.find_last_of('/'));

	if (!resource.empty() && resource.at(resource.size() - 1) != '/')
	{
		file_name = path.substr(path.find_last_of('/') + 1);
		if ((pos = file_name.find('.')) != std::string::npos)
			file_extension = file_name.substr(pos + 1);
	}
	if (file_name.empty() && directory.empty())
		directory = "/";
}

UrlParser::~UrlParser()
{
}

bool UrlParser::queryStringToParams(const std::string &string, std::map<std::string, std::string> &params)
{
	std::stringstream ss(string);
	std::string token, key, value;
	size_t pos;

	while (getline(ss, token, '&'))
	{
		if ((pos = token.find('=')) != std::string::npos)
		{
			key = token.substr(0, pos);
			value = token.substr(pos + 1);
			params.insert(std::make_pair(key, value));
		}
		else
			return false;
	}
	if (ss >> token)
		return false;
	return true;
}

std::ostream &operator<<(std::ostream &o, const UrlParser &rhs)
{
	o << "resource : " << rhs.resource << std::endl;
	o << "path : " << rhs.path << std::endl;
	o << "query_string : " << rhs.query_string << std::endl;
	o << "directory : " << rhs.directory << std::endl;
	o << "file_name : " << rhs.file_name << std::endl;
	o << "file_extension : " << rhs.file_extension << std::endl;
	o << "query params : " << std::endl;
	std::map<std::string, std::string>::const_iterator cit;
	for (cit = rhs.query_params.begin(); cit != rhs.query_params.end(); ++cit)
		o << "  " << cit->first << " : " << cit->second << std::endl;
	return o;
}
