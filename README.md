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

=============

Current state of the branch :

- The Webserv class parses the path to all the files in the www folder and its subfolders

- Then it parses all the servers, using the options in the configuration file and giving each server a unique name

- Each port is used to set a socket

- The main loop starts and waits for new connections

- We use the std::time() function to limit connections to 1 connection/sec/client

- The request is sent to the corresponding server

- The server splits the request into header and body

- The server uses the header to identify the requested location and method

- If the location is a url (ending with .html/.htm/.php), the server checks the url list and sends either the corresponding page or the 404 page

- In any other case, the server sends a sample "Hello from the server" page

- Back to the main loop

- Searching for localhost:[any_set_port]/kill properly stops the server, frees what needs to be freed, displays a message in the terminal and show a relevant page to the user in the browser

NB2 : I'm not sure about the allowed methods for the locations. Either I won't let the user look for direct url or I'll update any page linked to a location if methods other than GET are specified in the location block.