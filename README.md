# webserv

Naming convention :

- Variables, structures, namespaces : snake_case
- Functions : camelCase

==============================

Tasks:

- Starting configuration > Read and parse the configuration file and make the changes written in it > Done
- Request handling > open socket(s), poll() (or equivalent), listen, timeout... > In progress
- Reponse handling > Find the requested file if it exist, send errors if any, construct the html response and send it to the user > In progress
- CGI > chose a language (Python, c, c++...), make a program for uploading files, another to fill in a form

===============================

Current state of the branch :

- The Webserv class parses the path to all the files in the www folder and its subfolders

- Then it parses all the servers, using the options in the configuration file and giving each server a unique name

- Each port is used to set a socket

- The main loop starts and waits for new connections

- We use the std::time() function to limit connections to 1 connection/sec/client

- The request is sent to the corresponding server

- The server splits the request into header and body

- The server uses the header to identify the requested location and method

- If the location is a url (ending with .html/.htm/.php), the server checks the url list and sends either the corresponding page or the 404 page, then goes back to the main loop

- Searching for localhost:[any_set_port]/kill properly stops the server, frees what needs to be freed, displays a message in the terminal and show a relevant page to the user in the browser

- In any other case, the server sends a sample "Hello from the server" page and goes back to the main loop

- In any of those cases, the code and the message will be correct ("200 OK" if page found, "404 Not found" if not)

================================

NOTES :

- I'm quite satisfied with my select() implementation. No operation can be done unless select() said so. The program should be able to handle [ports * MAX_LISTEN] connections without failing. It will listen to all the pending requests, then pass them to the corresponding servers, which will handle them, and go back to listening. Select() makes sure the sockets (I/O) are always ready to listen/write.

- Instead of displaying messages on the terminal for every request received/handled, we could create a log file somewhere and write everything in it, with the time, the ip's, the server name and any other relevant information. > Done. But for some reason, I cannot add an ofstream to the Webserv class and keep it open always. It broke the program. Instead, I open and close the file each time a log needs to be done.

================================

TO DO :

- I need to find a way to apply the favicon and the stylesheet to the displayed pages

- I'm not sure about the allowed methods for the locations. Either I won't let the user look for direct url or I'll update any page linked to a location if methods other than GET are specified in the location block.