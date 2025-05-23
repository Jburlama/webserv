#include "../includes/Server.hpp"

Server::Server()
{
    throw std::logic_error("Provid port to listen on: Server(int port)");
}

// Create a socket for the server to listen on
// Configure the IP address
// Declare that the socket is ready to listen
Server::Server(int port)
{
    int yes;

    yes = 1;
    this->_addr.sin_family = AF_INET;
    this->_addr.sin_port = htons(port);
    this->_addr.sin_addr.s_addr = INADDR_ANY;

    this->_serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if (setsockopt(this->_serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) // Re-use socket if is already in use
		throw std::runtime_error("Server.cpp:21\n");
    if (bind(this->_serverfd, (const struct sockaddr *)&this->_addr, sizeof(this->_addr)) != 0)
		throw std::runtime_error("Server.cpp:23\n");
    if (listen(this->_serverfd, SOMAXCONN) != 0)
		throw std::runtime_error("Server.cpp:25\n");
}

Server::~Server()
{
    close(this->_serverfd);
}

// Wait for a client to send a request
int Server::get_client()
{
    int         client_socket;
    socklen_t   addr_len;

    addr_len = sizeof(struct sockaddr_in);
    client_socket = accept(this->_serverfd, (struct sockaddr *)&this->_addr, &addr_len);
    if (client_socket == -1)
    	throw std::runtime_error("Server.cpp:line:41\n");
    return client_socket;
}

int Server::getfd()
{
    return this->_serverfd;
}

// Handle multiple clients
void Server::client_multiplex()
{
    fd_set copy_socket_set;
    int     client_socket;
    FD_ZERO(&this->_socket_set);
    FD_SET(this->_serverfd, &this->_socket_set);
    
    while (42)
    {
        // because select is destructive
        copy_socket_set = this->_socket_set;

        if (select(FD_SETSIZE, &copy_socket_set, NULL, NULL, NULL) < 0)
		    throw std::runtime_error("Server.cpp:65\n");

        for (int i = 0; i < FD_SETSIZE; ++i)
        {
            if (FD_ISSET(i, &copy_socket_set)) // checking if fd is ready for reading
            {
                if (i == this->_serverfd)
                {
                    // this is a new connection
                    client_socket = this->get_client();
                    FD_SET(client_socket, &this->_socket_set); // add the client to set
                }
                else
                {
                    this->handle_request(i);
                    close(i); // close client socket fd
                    FD_CLR(i, &this->_socket_set); // rm from set
                }
            }
        }
    }
    FD_CLR(this->_serverfd, &this->_socket_set);
}

// send a response
// Recives the request from the client and prints to stdout
// send back a response with the client request
void Server::handle_request(int clientfd)
{
    char buffer[1024];

    memset(buffer, 0, sizeof(buffer));

    recv(clientfd, buffer, sizeof(buffer) - 1, 0); // Request
    send(clientfd, buffer, strlen(buffer), 0); // Response

    printf("%s\n", buffer); // log
}
