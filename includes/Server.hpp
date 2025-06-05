#pragma once
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>

class Server
{
    private:
        int                 _port;
        int                 _fd;
        struct sockaddr_in  _addr;
    public:
        Server();
        Server(int port);
        ~Server() {};

        int get_port() { return _port; };
        int get_fd() { return _fd; };
        struct sockaddr_in &get_addr() { return _addr; };
};
