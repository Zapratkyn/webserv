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

- The Webserv class parses the path to all the files in the www folder and its subfolders

- Then it parses all the servers, using the options in the configuration file and giving each server a unique name

- Each port is used to set a listening socket

- The main loop starts and waits for new connections

- The Webserv splits the request into header and body

- The server uses the header to identify the requested location and method

- If the location is a url (ending with .html/.htm/.php), the server checks the url list and sends either the corresponding page or the 404 page, then goes back to the main loop

- Searching for localhost:[any_set_port]/kill properly stops the server, frees what needs to be freed, displays a message in the terminal and show a relevant page to the user in the browser

- If you look for a set location (example, localhost:8083/loc, with the custom.conf file), you get a directory page. The links don't work yet.

- In any of those cases, the code and the message will be correct ("200 OK" if page found, "404 Not found" if not)

================================

NOTES :

- I'm quite satisfied with my select() implementation. No operation can be done unless select() said so. The program should be able to handle [ports * MAX_LISTEN] connections without failing. It will listen to the first pending request of each listening port, parse the whole request (header and body) and send it to the corresponding server. It will go through select() before any of those operations.

NB : For some reason, the previous implementation worked on my linux but not on the school's MAC's. I didn't go to school since my last modification. Therefore, I cannot guarantee it will work this time. I hope it will though...

- Instead of displaying messages on the terminal for every request received/handled, we could create a log file somewhere and write everything in it, with the time, the ip's, the server name and any other relevant information. > Done. But for some reason, I cannot add an ofstream to the Webserv class and keep it open always. It broke the program. Instead, I open and close the file each time a log needs to be done.

================================

TO DO :

- I need to find a way to apply the favicon and the stylesheet to the displayed pages

- I'm not sure about the allowed methods for the locations. Either I won't let the user look for direct url or I'll update any page linked to a location if methods other than GET are specified in the location block.

- Manage the links

- Redirections

- Use the www as the root of everything. Meaning the user CANNOT access anything in the main folder AND start any requested location with "./www/"

- CGI !!!