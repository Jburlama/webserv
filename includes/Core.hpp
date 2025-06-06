#pragma once
#include <cstdio>
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <fstream>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <cstdlib>

#include "webserv.hpp"
#include "HttpResponse.hpp"
#include "Client.hpp"
#include "Server.hpp"
#include "File.hpp"
#include "Log.hpp"

#define BUFFER_SIZE 40000

class Core
{
    private:
        std::map<int, Server>   _servers;   // Track servers by fd
        std::map<int, Client>   _clients;   // Track clients by fd
        time_t                  _timeout;   // Global timeout in seconds
        fd_set                  _read_set;  // Separate read FD set
        fd_set                  _write_set; // Separate write FD set
        bool                    _client_connection;
        int                     _biggest_fd;
    
    public:
        Core(); // Don't call default
        Core(std::vector<int> ports);
        ~Core() {};

        void    get_client(int server_fd);
        bool    get_connection_state() {return this->_client_connection;};

        void    build_request(int client_fd);
        void    build_response(int client_fd);
        void    handle_write(int client_fd);
        void    close_client(const int fd);
        bool    check_timeouts(int fd);
        void    client_multiplex();
        void    set_connection_state(bool state) {this->_client_connection = state;};
};
