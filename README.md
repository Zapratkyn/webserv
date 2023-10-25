# webserv

Naming convention :

- Variables, structures, namespaces : snake_case
- Functions : camelCase

Tasks:

- Starting configuration > Read and parse the configuration file and make the changes written in it > Gilles
- Request handling > open socket(s), poll() (or equivalent), listen, timeout... >
- Reponse handling > Find the requested file if it exist, send errors if any, construct the html response and send it to the user
- CGI > chose a language (Python, c, c++...), make a program for uploading files, another to fill in a form

Notes about request handling :

Depending on the browser, the header will not be structured exactly the same
Firefox and Safari ask for a specific location in the first line (GET [location] HTTP/1.1)
Chrome and Brave ask for a specific location later, in a line called 'Referer' (Referer: http://localhost:[port][location])