#include "../includes/Server.hpp"

Server::Server()
:_port(-1),_fd(-1)
{
    memset(&_addr, 0, sizeof(_addr));
}

// Create a socket for the server to listen on
// Configure the IP address
// Declare that the socket is ready to listen
Server::Server(int port)
:_port(port)
{
    int     yes;

    yes = 1;
    this->_addr.sin_family = AF_INET;
    this->_addr.sin_port = htons(port);
    this->_addr.sin_addr.s_addr = INADDR_ANY;

    this->_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(this->_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) // Re-use socket if is already in use
        throw std::runtime_error("Server.cpp:23\n");
    if (bind(this->_fd, (const struct sockaddr *)&this->_addr, sizeof(this->_addr)) != 0)
        throw std::runtime_error("Server.cpp:25\n");
    if (listen(this->_fd, SOMAXCONN) != 0)
        throw std::runtime_error("Server.cpp:28\n");
}
