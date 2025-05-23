#pragma once
#include <stdexcept>
#include <sys/socket.h>
#include <sys/select.h>

#include "webserv.hpp"

class Server
{
    private:
        int _serverfd;
        struct sockaddr_in _addr;
        fd_set  _socket_set;
    public:
        Server(); // Don't call default
        Server(int port);
        ~Server();
        int get_client();
        int getfd();
        void client_multiplex();
        void handle_request(int client_socket);
};
