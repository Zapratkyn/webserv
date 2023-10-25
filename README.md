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

The request is kinda random.

Sometimes you'll get the location on the first line

Sometimes you'll get it later on a line called 'Referer'

We need to check both every time
