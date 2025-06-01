#pragma once
#include <cstdio>
#include <stdexcept>
#include <sys/socket.h>
#include <sys/select.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>

#include "webserv.hpp"
#include "HttpResponse.hpp"
#include "Client.hpp"
#include "Server.hpp"

class Core
{
    private:
        std::map<int, Server>   _servers;   // Track servers by fd
        std::map<int, Client>   _clients;   // Track clients by fd
        time_t                  _timeout;   // Global timeout in seconds
        fd_set                  _read_set;  // Separate read FD set
        fd_set                  _write_set; // Separate write FD set
    
    public:
        Core(); // Don't call default
        Core(std::vector<int> ports);
        ~Core() {};

        void    get_client(int server_fd);

        void    handle_read(int client_fd);
        void    handle_write(int client_fd);
        void    close_client(int fd);
        void    check_timeouts();
        void    client_multiplex();
};
