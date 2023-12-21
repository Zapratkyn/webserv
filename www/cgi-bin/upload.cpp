#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <sys/socket.h>
#include <stdlib.h>
#include <unistd.h>

int ft_stoi(std::string);
std::string successPage();
std::string getFilename(std::string &);
void trim(std::string &);
std::string getBoundary(std::string &);
void fileOnly(std::string &, std::string &);
int findStartOfFile(std::string &, std::string &);

int main(int argc, char **argv, char **env)
{
	std::ifstream infile("tmp", std::ifstream::in | std::ifstream::binary);
	std::ofstream outfile;
	std::string file, filename, boundary, message;
	std::string socket(argv[0]);
	char c;
	int pos, sock;

	while (true)
	{
		if (infile.eof())
			break;
		infile.read(&c, 1);
		file.append(1, c);
	}
	infile.close();

	sock = ft_stoi(&socket[socket.find_first_of("=") + 1]);

	boundary = getBoundary(file);
	boundary.insert(0, "--");
	filename = getFilename(file);
	filename.insert(0, "www/uploads/");
	pos = findStartOfFile(file, boundary);
	file.erase(0, pos);
	file.erase(file.find(boundary) - 4, file.npos);

	outfile.open(filename, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);
	outfile << file;
	outfile.close();
	unlink("tmp");

	message = successPage();

	send(sock, message.c_str(), message.size(), 0);

	return 0;
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

std::string successPage()
{
	std::string html, message = "HTTP/1.1 200 OK\r\nConnection: keep-alive\r\nContent-Length: \r\nContent-Type: text/html\r\n\r\n";
	std::stringstream ss;
	ss << "<!DOCTYPE html>"
	      "<html>\n"
	      "<head><title>"
		  "Upload successful"
	      "</title></head>\n"
	             "<body>\n"
	             "<center><h1>"
				 "You file has been successfully uploaded !"
	      "</h1></center>\n"
	             "</body>\n"
	             "</html>";
	html = ss.str();
	message.insert(message.find("\r\nContent-Type"), ft_to_string(html.size()));
	message.append(html);
	return message;
}

std::string getFilename(std::string &file)
{
	std::string result;

	result = &file[file.find("filename=\"") + 10];
	result = result.substr(0, result.find('"'));

	return result;
}

std::string getBoundary(std::string &file)
{
	std::string result;

	result = &file[file.find("boundary=") + 9];
	result = result.substr(0, result.find("\r\n"));

	return result;
}

void fileOnly(std::string &file, std::string &boundary)
{
	file = &file[file.find("\r\n\r\n")];
	file = &file[file.find(boundary) + boundary.size()];
	file = &file[file.find("\r\n\r\n") + 4];
	file = file.substr(0, file.find(boundary));
}

int findStartOfFile(std::string &file, std::string &boundary)
{
	int pos = 0;

	pos = file.find(boundary) + boundary.size();
	pos = file.find("\r\n\r\n", pos) + 4;

	return pos;
}