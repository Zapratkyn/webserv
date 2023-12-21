#include <iostream>
#include <fstream>
#include <sstream>
#include <map>
#include <sys/socket.h>

int ft_stoi(std::string);
std::string errorPage();
std::map<std::string, std::string> getFileTypes();
std::string ft_to_string(int nb);

int main(int argc, char **argv, char **env)
{
	std::ifstream infile;
	std::string socket(argv[0]);
	std::string buff_size(argv[1]);
	std::string filename(argv[2]);
	std::string body;
	std::string message = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: \r\nContent-Type: \r\n\r\n";
	std::map<std::string, std::string> types = getFileTypes();
	int sock, size, pos;
	char c;

	filename = filename.substr(1, filename.rfind('/') - 1);
	filename.insert(0, "www/uploads/");

	infile.open(filename, std::ifstream::in | std::ifstream::binary);
	if (infile.fail())
		message = errorPage();
	else
	{
		sock = ft_stoi(&socket[socket.find('=') + 1]);
		size = ft_stoi(&buff_size[buff_size.find('=') + 1]);
		while (!infile.eof())
		{
			infile.read(&c, 1);
			body.append(1, c);
		}
		message.insert(message.find("\r\nContent-Type"), ft_to_string(body.size()));
		message.insert(message.find("\r\n\r\n"), types[&filename[filename.rfind('.') + 1]]);
		message.append(body);
	}

	send(sock, message.c_str(), message.size(), 0);
}

std::map<std::string, std::string> getFileTypes()
{
	std::map<std::string, std::string> types;
	types["html"] = "text/html";
	types["htm"] = "text/html";
	types["php"] = "text/html";
	types["js"] = "application/javascript";
	types["pdf"] = "application/pdf";
	types["json"] = "application/json";
	types["xml"] = "application/xml";
	types["zip"] = "application/zip";
	types["wav"] = "audio/x-wav";
	types["gif"] = "image/gif";
	types["jpeg"] = "image/jpeg";
	types["jpg"] = "image/jpeg";
	types["png"] = "image/png";
	types["ico"] = "image/x-icon";
	types["css"] = "text/css";
	types["csv"] = "text/csv";
	types["mp4"] = "video/mp4";
	return types;
}

int ft_stoi(std::string str)
{
	int result = 0;
	int size = str.size();

	for (int i = 0; i < size; i++)
		result = result * 10 + str[i] - '0';

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

std::string errorPage()
{
	std::string html, message = "HTTP/1.1 500 Internal Server Error\r\nConnection: keep-alive\r\nContent-Length: \r\nContent-Type: text/html\r\n\r\n";
	std::stringstream ss;
	ss << "<!DOCTYPE html>"
	      "<html>\n"
	      "<head>"
		  	// "<meta charset=\"UTF-8\">\n"
   		  	// "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\n"
		  	// "<link rel=\"shortcut icon\" type=\"image/x-icon\" href=\"/assets/favicon.ico\">\n"
    	  	// "<link rel=\"stylesheet\" type=\"text/css\" href=\"/assets/stylesheet.css\">\n"
		  	"<title>Upload failed</title>\n"
	      "</head>\n"
	             "<body>\n"
	             "<center><h1>"
				 "500 Internal Server Error"
	      "</h1></center>\n"
	             "</body>\n"
	             "</html>";
	html = ss.str();
	message.insert(message.find("\r\nContent-Type"), ft_to_string(html.size()));
	message.append(html);
	return message;
}

// void chunkReponse(std::string message)
// {

// 	size_t pos1 = _message.find("Content-Length"), pos2 = _message.find("\r\nContent-Type"), copied = 0;
// 	std::ifstream file(_resource_path, std::ifstream::binary);
// 	std::stringstream converter;
// 	std::vector<char> buffer;
// 	char c;
	
// 	_message.replace(pos1, pos2 - pos1, "Transfer-Encoding: chunked");
// 	_body = "";

// 	for (int i = 0; i < BUFFER_SIZE; i++)
// 	{
// 		if (file.eof())
// 			break;
// 		copied++;
// 		file.read(&c, 1);
// 		buffer.push_back(c);
// 	}

// 	while (copied > 0)
// 	{
// 		converter.str("");
// 		converter << std::hex << copied;
// 		_body.append(converter.str());
// 		_body.append("\r\n");
// 		for(std::vector<char>::iterator it = buffer.begin(); it != buffer.end(); it++)
// 			_body.append(1, *it);
// 		_body.append("\r\n");
// 		buffer.clear();
// 		copied = 0;
// 		for (int i = 0; i < BUFFER_SIZE; i++)
// 		{
// 			if (file.eof())
// 				break;
// 			copied++;
// 			file.read(&c, 1);
// 			buffer.push_back(c);
// 		}
// 	}

// 	file.close();
// 	_body.append("0\r\n\r\n");
// }