#pragma once
#include <stdexcept>
#include <sys/socket.h>
#include <sys/select.h>
#include <fstream>
#include <iostream>
#include <string>

#include "webserv.hpp"

class Core
{
    private:
        int _serverfd;
        struct sockaddr_in _addr;
        fd_set  _socket_set;
    public:
        Core(); // Don't call default
        Core(int port);
        ~Core();
        int get_client();
        int getfd();
        void client_multiplex();
        void handle_request(int client_socket);
};
