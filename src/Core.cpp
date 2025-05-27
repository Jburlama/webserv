#include "../includes/Core.hpp"

Core::Core()
{
    throw std::logic_error("Provid port to listen on: Core(int port)");
}

// Create a socket for the server to listen on
// Configure the IP address
// Declare that the socket is ready to listen
Core::Core(std::vector<int> ports)
{
    Listen  listen_on;
    int     yes;

    yes = 1;
    for (std::vector<int>::iterator it = ports.begin(); it != ports.end(); ++it)
    {
        listen_on.addr.sin_family = AF_INET;
        listen_on.addr.sin_port = htons(*it);
        listen_on.addr.sin_addr.s_addr = INADDR_ANY;

        listen_on.serverfd = socket(AF_INET, SOCK_STREAM, 0);
        if (setsockopt(listen_on.serverfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) == -1) // Re-use socket if is already in use
            throw std::runtime_error("Core.cpp:21\n");
        if (bind(listen_on.serverfd, (const struct sockaddr *)&listen_on.addr, sizeof(listen_on.addr)) != 0)
            throw std::runtime_error("Core.cpp:23\n");
        if (listen(listen_on.serverfd, SOMAXCONN) != 0)
            throw std::runtime_error("Core.cpp:25\n");
        this->_listen.insert(this->_listen.end(), listen_on);
    }
}

Core::~Core()
{
    for (std::vector<Listen>::iterator it = this->_listen.begin(); it != this->_listen.end(); ++it)
        close(it->serverfd);
}

// Wait for a client to send a request
int Core::get_client(int server_fd)
{
    int         client_socket;
    socklen_t   addr_len;

    addr_len = sizeof(struct sockaddr_in);
    for (std::vector<Listen>::iterator it = this->_listen.begin(); it != this->_listen.end(); ++it)
    {
        if (it->serverfd == server_fd)
            client_socket = accept(server_fd, (struct sockaddr *)&it->addr, &addr_len);
    }
    if (client_socket == -1)
    	throw std::runtime_error("Core.cpp:line:41\n");
    return client_socket;
}

bool Core::is_server_fd(int fd)
{
    for (std::vector<Listen>::iterator it = this->_listen.begin(); it != this->_listen.end(); ++it)
    {
        if (it->serverfd == fd)
            return true;
    }
    return false;
}

// Handle multiple clients
void Core::client_multiplex()
{
    fd_set  copy_socket_set;
    int     client_socket;
    FD_ZERO(&this->_socket_set);
    for (std::vector<Listen>::iterator it = this->_listen.begin(); it != this->_listen.end(); ++it)
        FD_SET(it->serverfd, &this->_socket_set);
    
    while (42)
    {
        // because select is destructive
        copy_socket_set = this->_socket_set;

        if (select(FD_SETSIZE, &copy_socket_set, NULL, NULL, NULL) < 0)
		    throw std::runtime_error("Core.cpp:65\n");

        for (int i = 0; i < FD_SETSIZE; ++i)
        {
            if (FD_ISSET(i, &copy_socket_set)) // checking if fd is ready for reading
            {
                if (is_server_fd(i))
                {
                    // this is a new connection
                    client_socket = this->get_client(i);
                    FD_SET(client_socket, &this->_socket_set); // add the client to set
                }
                else
                {
                    this->handle_message(i);
                    FD_CLR(i, &this->_socket_set); // rm from set
                }
            }
        }
    }
    for (std::vector<Listen>::iterator it = this->_listen.begin(); it != this->_listen.end(); ++it)
        FD_CLR(it->serverfd, &this->_socket_set);
}

// send a response
// Recives the request from the client and prints to stdout
// send back a response with the client request
void Core::handle_message(int clientfd)
{
    char            buffer[10240];
    std::string     request_method;

    memset(buffer, 0, sizeof(buffer));

    if (recv(clientfd, buffer, sizeof(buffer) - 1, 0) == -1) // Request
    	throw std::runtime_error("Core.cpp:line:116\n");

    HttpRequest http_request(buffer);
    HttpResponse http_response(http_request, clientfd);

    close(clientfd); // close client socket fd
    std::cout << "\nResponse Sended\n\n";
}
