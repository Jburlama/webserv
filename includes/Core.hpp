#pragma once
#include <stdexcept>
#include <sys/socket.h>
#include <sys/select.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

#include "webserv.hpp"

struct Listen {
    int                 serverfd;
    struct sockaddr_in  addr;
};

class Core
{
    private:
        std::vector<Listen> _listen;
        fd_set              _socket_set;
    public:
        Core(); // Don't call default
        Core(std::vector<int> ports);
        ~Core();
        int get_client(int server_fd);
        bool is_server_fd(int fd);
        void client_multiplex();
        void handle_message(int client_socket);
};
