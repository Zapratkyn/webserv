#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <map>
#include <sys/socket.h>

int ft_stoi(std::string);
void errorPage(std::string &);
void successPage(std::string &);
std::map<std::string, std::string> getExtensions();
std::string getFilename(std::string &);
void trim(std::string &);
std::string getBoundary(std::string &);
void fileOnly(std::string &, std::string &);
int findStartOfFile(std::string &);

int main(int argc, char **argv, char **env)
{
	std::ifstream infile(argv[0], std::ifstream::in | std::ifstream::binary);
	std::ofstream outfile;
	std::string file, filename, boundary, size;
	std::string message = "Connection: keep-alive\r\nContent-Length: \r\nContent-Type: text/html\r\n\r\n";
	std::string body_size(env[0]), socket(env[1]);
	std::map<std::string, std::string> extensions = getExtensions();
	char c;
	int file_size, pos;

	while (true)
	{
		if (infile.eof())
			break;
		infile.read(&c, 1);
		file.append(1, c);
	}

	std::cout << file << std::endl;

	size = &file[file.find("Content-Length: ") + 16];
	size = size.substr(0, size.find_first_of("\r\n"));
	file_size = ft_stoi(size);

	if (file_size > ft_stoi(&body_size[body_size.find_first_of("=") + 1]))
		errorPage(message);
	else
	{
		boundary = getBoundary(file);
		boundary.insert(0, "--");
		filename = getFilename(file);
		filename.insert(0, "../uploads/");
		pos = findStartOfFile(file);
		infile.seekg(0, infile.beg);
		infile.seekg(pos);
		file.clear();
		for (int i = 0; i < file_size; i++)
		{
			infile.read(&c, 1);
			std::cout << c;
			file.append(1, c);
		}

		std::cout << file << std::endl;

		std::cout << file << std::endl;

		outfile.open(filename, std::ofstream::out | std::ofstream::trunc | std::ofstream::binary);

		outfile << file;

		outfile.close();

		successPage(message);
	}

	send(ft_stoi(&socket[socket.find_first_of("=") + 1]), message.c_str(), message.size(), 0);

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

void errorPage(std::string &message)
{
	std::string html;
	std::stringstream ss;
	ss << "<!DOCTYPE html>"
	      "<html>\n"
	      "<head><title>"
	      << 413
	      << " Content Too Large"
	             "</title></head>\n"
	             "<body>\n"
	             "<center><h1>"
	      << 413
	      << " Content Too Large"
	             "</h1></center>\n"
	             "</body>\n"
	             "</html>";
	html = ss.str();
	message.insert(0, "HTTP/1.1 413 Content Too Large\r\n");
	message.insert(message.find("\r\nContent-Type"), ft_to_string(html.size()));
}

void successPage(std::string &message)
{
	std::string html;
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
	message.insert(0, "HTTP/1.1 200 OK\r\n");
	message.insert(message.find("\r\nContent-Type"), ft_to_string(html.size()));
}

std::map<std::string, std::string> getExtensions()
{
	std::map<std::string, std::string> ext;
	ext["text/html"] = ".html";
	ext["text/htm"] = ".htm";
	ext["audio/x-wav"] = ".wav";
	ext["image/gif"] = ".gif";
	ext["image/jpeg"] = ".peg";
	ext["image/jpg"] = ".jpeg";
	ext["image/png"] = ".png";
	ext["image/x-icon"] = ".ico";
	ext["text/css"] = ".css";
	ext["text/csv"] = ".csv";
	ext["video/mp4"] = ".mp4";
	ext["application/pdf"] = ".pdf";
	return ext;
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

int findStartOfFile(std::string &file)
{
	int pos = 0;

	pos = file.find("\r\n\r\n") + 4;
	pos = file.find("\r\n\r\n", pos) + 4;

	return pos;
}