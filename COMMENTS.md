#### Misc

- I've added a Clang-format file to automate code formatting, `.clang-format` in root directory.
- Makefile has a debug option. To use it, run `make debug FLAGS="-<flag1> -<flag2> ..."`

#### Useful commands

- `lsof -nP -iTCP -sTCP:LISTEN | grep IPv4` to check which sockets are listening

#### Config file

- listen directive
    - the subject asks to be able to choose port and host of each server. In your code only the port can be set, and the
      host defaults to localhost.
    - I've changed it a bit to be able to set port and host, according to what NGINX does (
      see [NGINX docs on listen](http://nginx.org/en/docs/http/ngx_http_core_module.html#listen)) :
      ``` cpp
      // Server class
      typedef std::pair<std::string, std::string> host_port_type;
      std::vector<host_port_type> _endpoints;
      std::vector<int> _sockets;
      std::vector<struct sockaddr_in> _sock_addrs;
      ``` 
- server_name directive
    - can contain multiple values per line and the option can also occur more than once
    - we also need to know which server is the first in the list (the default server), and the sorting feature of map of
      strings prevents that
        - Changes :
          ``` cpp
          // Server Class
          std::vector<std::string> _server_names;
          int _serverID;
        
          // Webserver Class
          std::map<int, Server *> _server_list; // a bit of an overkill 
          //std::vector<Server> _server_list; 
          ```
- redirect directive
    - redirects shouldn't be handled using a file, but as directive for a server in the config file according to the
      subject (sth like rewrite or return in nginx, but without the regex stuff to keep it simple)


#### Links
- [How nginx processes a request](http://nginx.org/en/docs/http/request_processing.html)
  - How to decide if server_name or IP address and port define which server will process the request