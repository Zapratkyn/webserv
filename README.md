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

TO DO :

- I need to find a way to apply the favicon and the stylesheet to the displayed pages

- I'm not sure about the allowed methods for the locations. Either I won't let the user look for direct url or I'll update any page linked to a location if methods other than GET are specified in the location block.

- The PDF states we need to get through select for every I/O operation. The current program does it only for reading requests. I'll need to use a second fd_set. The potential problem is that I could stack request and never answer any of them as long as new request come in...

Potential solution : 
> Keep track of the biggest listening socket int. 
> Use select() The first time, it should detect only the very first request and go to newConnection() right away. Other times, new connections might have stacked on any listening socket by the time we handle the previous ones.
> In newConnection(), stack all the incoming request sockets in the 2nd fd_set and in a vector<int, t_request> and go back to the main loop without handling anything.
> Select() again and compare the return value to the biggest listening socket int to determine if we need to handle stacked requests or get new ones
> Handle stacked requests one by one, removing the sockets from the 2nd fd_set and the vector<int, t_request>

NB : I'm still not quite sure about it because select() is used not on every operation but alternatively on in and out series of operations. But since select() browse through all the fd's and sets to 1 all of those which are available for in or out operation depending on the fd_set they are in, I suppose we could consider all the writers went through select() with this solution.