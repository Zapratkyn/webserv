# webserv

Warning :

If during testing you get a "Couldn't bind socket" error, just keep trying to run the program. A listening socket stays unavailable for a short period after being used by a program, even if you close it with the kill method.

==============================

Naming convention :

- Variables, structures, namespaces : snake_case
- Functions : camelCase

==============================

Tasks:

- Starting configuration > Read and parse the configuration file and make the changes written in it > Done
- Request handling > open socket(s), poll() (or equivalent), listen, timeout... > Done (I think...)
- Reponse handling > Find the requested file if it exist, send errors if any, construct the html response and send it to the user > In progress
- CGI > chose a language (Python, c, c++...), make a program for uploading files, another to fill in a form

===============================

Current state of the branch :

- The Webserv class parses the path to all the files and folders in the www folder and its subfolders

- Then it parses all the servers, using the options in the configuration file and giving each server a unique name

- Each port is used to set a listening socket

- The main loop starts and waits for new connections

- All pending requests are stacked in a list. An error (500) is sent to any client after the MAX_LISTEN is reached on a socket

- The webserv identify the requested server and the client for each request

- The Webserv splits the request into header and body

- The corresponding server uses the header to identify the requested location and method

- If the location is a url (ending with .html/.htm/.php), the server checks the url list and sends either the corresponding page or the 404 page, then goes back to the main loop

- If the location is directory or a location with autoindex setup, the server sends ./dir.html to the client, filled with links to any file or folder found in the directory

- Searching for localhost:[any_set_port]/kill properly stops the server, frees what needs to be freed, displays a message in the terminal and in the log and shows a relevant page to the user in the browser

- In any of those cases, the code and the message will be correct ("200 OK" if page found, "404 Not found" if not), the favicon will be used in the thumbnail and the stylesheet will be applied to the current page

================================

NOTES :

- The select implementation could be improved even more.

- The start/stop messages and the requests in/out are logged in the webserv.log file

- The browser keeps the favicon associated the the url in cache. Meaning it will not request it to the server again. The same is not true for the stylesheet.

================================

TO DO :

- I'm not sure about the allowed methods for the locations. Either I won't let the user look for direct url or I'll update any page linked to a location if methods other than GET are specified in the location block.

- CGI !!!