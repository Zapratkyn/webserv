#include <iostream>
#include <fstream>

int main()
{
    std::ifstream ifs("./www/dir.html");
    std::string html = "", buffer, loc = "./www";
    int spot;

    while (!ifs.eof())
	{
		getline(ifs, buffer);
		html.append(buffer);
		html.append("\n");
	}

    std::cout << html << std::endl;

    spot = html.find("</tr>") + 5;
    
    html.insert(spot, "\n\t<tr>\n\t\t<td><a href=");
    spot += 21;
    html.insert(spot++, 1, '"');
	html.insert(spot, loc);
	spot += (loc.size());
    html.insert(spot++, 1, '"');
	html.insert(spot, ">Parent directory</a></td>\n\t</tr>\n");

    std::cout << html << std::endl;
    return 0;
}