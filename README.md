# webserv

Naming conventions :

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

- It also parse the various icons used for dir.html and the favicon

- Then it parses all the servers, using the options in the configuration file and giving each server a unique name

- Each port is used to set a listening socket

- The main loop starts and waits for new connections

- All pending requests are stacked in a list.

- The webserv identify the requested server and the client for each request

- The Webserv splits the request into header and body

- The corresponding server uses the header to identify the requested location and method

- If pselect() fails at step 2 or 3, it will send a error (500) to all the clients in the request list. It will go back to step 1 in any case

- If the location is a url (ending with .html/.htm/.php), the server checks the url list and sends either the corresponding page or the 404 page, then goes back to the main loop

- If the location is directory or a location with autoindex setup, the server sends ./dir.html to the client, filled with links to any file or folder found in the directory

- If the requested url doesn't match a file nor a location, it will check the redirection list and change the http code accordingly (308)

- Searching for localhost:[any_set_port]/kill properly stops the server, frees what needs to be freed, displays a message in the terminal and in the log and shows a relevant page to the user in the browser

- In any of those cases, the code and the message will be correct ("200 OK" if page found, "404 Not found" if not), the favicon will be used in the thumbnail and the stylesheet will be applied to the current page

================================

NOTES :

- The select implementation could be improved even more.

- The start/stop messages and the requests in/out are logged in the webserv.log file

- The browser keeps the favicon associated with the url in cache. Meaning it will not request it to the server again. The same is not true for the stylesheet.

================================

TO DO :

- Some types of media (So far, I tried only pdf) struggle to be sent via binary opening mode. Icons work most of the time

- CGI !!!