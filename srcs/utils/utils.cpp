#include "../../include/utils/utils.hpp"

/*
Because of the -std=c++98 flag, we can't use std::string::pop_back(), std::stoi() and std::to_string()
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

    if (str.size() && str.find_first_not_of(" \t") != std::string::npos)
    {
        result = &str[str.find_first_not_of(" \t")];
        while (result[result.size() - 1] == ' ' || result[result.size() - 1] == '\t')
            result = ft_pop_back(result);
    }
    return result;
}

int ft_stoi(std::string str)
{
	int result = 0;
	int size = str.size();

	for (int i = 0; i < size; i++)
		result = result * 10 + str[i] - '0';

	return result;
}

void log(std::string line, std::string client, std::string url, int type)
{
	time_t tm = std::time(NULL);
	char* dt = ctime(&tm);
	std::string odt = ft_pop_back(dt);
	std::ofstream log_file;

	log_file.open("./webserv.log", std::ofstream::app);

	log_file << odt << " - ";

	switch (type) {
	case 0:
		log_file << line << "\n";
		break;
	case 1:
		log_file << client << ": " << line << "\n";
		break;
	case 2:
		log_file << ": Request (" << url << ") received from " << client << "\n";
		break;
	case 3:
		log_file << ": Reponse (" << &url[1] << ") sent to " << client << "\n";
		break;
	}
	
	log_file.close();
}

bool sendText(t_request &request)
{
	std::ifstream 	ifs;
	std::string		html = "", buffer, extension = &request.url[request.url.find_last_of(".") + 1];
	// We start our response by the http header with the right code
	std::string 	result = "HTTP/1.1 ";

	ifs.open(request.url.c_str());
	if (ifs.fail())
		return false;

	result.append(request.code);
	result.append("\nContent-Type: text/");
	result.append(getContentType(extension));
	result.append("\nContent-Length: ");
	
	while (!ifs.eof())
	{
		getline(ifs, buffer);
		html.append(buffer);
		html.append("\n");
	}
	ifs.close();
	result.append(ft_to_string(html.size())); // We append the size of the html page to the http response
	result.append("\n\n"); // The http response's header stops here
	result.append(html); // The http reponse body (html page)

	if (DISPLAY_HTML)
		std::cout << result << std::endl;

	write(request.socket, result.c_str(), result.size());
	return true;
}

void sendFile(t_request &request)
{
	std::ifstream 	ifs(request.url.c_str(), std::ifstream::binary);
	std::string		file = "", buffer, extension = &request.url[request.url.find_last_of(".") + 1];
	std::string 	result = "HTTP/1.1 200 OK\nContent-Type: ";

	result.append(getContentType(extension));
	result.append("\nContent-Length: ");

	while (!ifs.eof())
	{
		getline(ifs, buffer);
		file.append(buffer);
		file.append("\n");
	}
	result.append(ft_to_string(file.size()));
	result.append("\n\n");
	result.append(file);

        ssize_t bytes_written = write(request.socket, result.c_str(), result.size());
        std::cout << "bytes written : " << bytes_written << std::endl;
        std::cout << "size : " << result.size() << std::endl;

	ifs.close();
}

void sendError(int error, int socket)
{
	std::string 	header = "HTTP/1.1 \nContent-Type: text/html\nContent-Lenght: ";
	std::string		body = "<!DOCTYPE html>\n", result;

	body.append("<html lang=\"fr\">\n<head>\n\t<meta charset=\"UTF-8\">\n");
	body.append("\t<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n");
	body.append("\t<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/icons/favicon.ico\">\n");
	body.append("\t<link rel=\"stylesheet\" type=\"text/css\" href=\"/others/stylesheet.css\">\n");
	body.append("\t<title>Bad Request</title>\n</head>\n\n<body>\n");
	body.append("\t<section id=\"error-page-section\">\n");
	body.append("\t\t<div class=\"err-page-container\">\n");
	body.append("\t\t\t<h1></h1>\n\t\t\t<h3></h3>\n\t\t\t<p>Sorry, .</p>\n");
	body.append("\t\t\t<p>Please return to the <a href=\"/www/pages/index.html\">home page</a>.</p>\n");
	body.append("\t\t</div>\n\t</section>\n</body>\n</html>");

	switch (error) {
		case 400:
			header.insert(header.find("\nContent-Type"), "400 Bad Request");
			body.insert(body.find("</h1>"), "400");
			body.insert(body.find("</h3>"), "Bad Request");
			body.insert(body.rfind(',') + 2, "the request's syntax was incorrect");
			break;
		case 404:
			header.insert(header.find("\nContent-Type"), "404 Not found");
			body.insert(body.find("</h1>"), "404");
			body.insert(body.find("</h3>"), "Not Found");
			body.insert(body.rfind(',') + 2, "the page you are looking for was not found");
			break;
		case 500:
			header.insert(header.find("\nContent-Type"), "500 Internal Server Error");
			body.insert(body.find("</h1>"), "500");
			body.insert(body.find("</h3>"), "Internal Server Error");
			body.insert(body.rfind(',') + 2, "an internal error has occured");
			break;
	}

	result = header.append(ft_to_string(body.size()));
	result.append("\n\n");
	result.append(body);

	write(socket, result.c_str(), result.size());
}

std::string getContentType(std::string extension)
{
	if (extension == "css")
		return "css";
	else if (extension == "html" || extension == "htm")
		return "html";
	else if (extension == "ico")
		return ("image/x-icon");
	else if (extension == "pdf")
		return ("application/pdf");
	return "";
}