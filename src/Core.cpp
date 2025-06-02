#include "../includes/Core.hpp"
#include <cstdio>
#include <cstring>
#include <vector>

Core::Core()
{
    throw std::logic_error("Provid port to listen on: Core(int port)");
}

// Inicialize the servers by adding to the servers map
// Each server is accessible by its file descriptor
Core::Core(std::vector<int> ports)
:_timeout(75) // Connetion timeout 75 seconds
{   
    FD_ZERO(&this->_read_set);
    FD_ZERO(&this->_write_set);
    for (std::vector<int>::iterator it = ports.begin(); it != ports.end(); ++it)
    {
        Server server(*it);

        this->_servers.insert(std::make_pair(server.get_fd(), server));
        FD_SET(server.get_fd(), &this->_read_set); // Add server fd to the read set
    }
}


// Accepts client connection to the given server
void Core::get_client(int server_fd)
{
    socklen_t                       addr_len;
    int                             client_fd;
    std::map<int, Server>::iterator serv_it;

    addr_len = sizeof(struct sockaddr_in);
    serv_it = this->_servers.find(server_fd);
    if (serv_it != this->_servers.end())
    {
        client_fd = accept(server_fd, (struct sockaddr *)&(serv_it->second.get_addr()), &addr_len);
        if (client_fd == -1)
        	throw std::runtime_error("Core.cpp:40\n");
        else if (client_fd >= 0)
        {
            std::cout << "Connetion made with server: " << server_fd << ", client: " << client_fd << "\n";
            this->_clients.insert(std::make_pair(client_fd, Client(client_fd)));
            std::cout << "Client: " << client_fd << " Added to read fd set\n";
            FD_SET(client_fd, &this->_read_set); // add the client to read set
        }
    }
}

// Handle multiple clients
void Core::client_multiplex()
{
    fd_set          read_ready;
    fd_set          write_ready;
    struct timeval  tv;
    int             ready; // Select return the amount of fd is ready

    //  sec timeout, so that select doesn't block;
    tv.tv_sec = 5;
    tv.tv_usec = 0;
    while (42)
    {
        // Copy because select is destructive and will change the sets
        read_ready  = this->_read_set;
        write_ready = this->_write_set;
        
        ready = select(FD_SETSIZE, &read_ready, &write_ready, NULL, &tv);
        if (ready < 0)
            throw std::runtime_error("Core.cpp:154\n");
        if (ready == 0) // No fd is ready, timeout has expired
            continue ;

        // Check servers
        for (std::map<int, Server>::iterator it = this->_servers.begin(); it != this->_servers.end(); ++it)
        {
            if (FD_ISSET(it->first, &read_ready)) // Check if fd is still present in the set
            {
                std::cout << "Making Connetion to server: fd " << it->first << "\n";
                this->get_client(it->first); // Accept a client for the serve
            }
        }

        // Check clients
        for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
        {
            if (FD_ISSET(it->first, &this->_read_set) && it->second.get_state() == READING) // Check if client is in read set
            {
                std::cout << "Reading from client: " << it->first << "\n";
                this->handle_read(it->first); // Call recv()
            }
            if (FD_ISSET(it->first, &this->_write_set)) // Check if client is in write set
            {
                std::cout << "Writing to Client: " << it->first << "\n";
                this->handle_write(it->first); // Call send().
            }
        }
        // Close a client connection if timeout as exceeded
        this->check_timeouts();
    }
}

void Core::handle_read(int client_fd)
{                 
    char    buffer[400000];
    ssize_t bytes;

    memset(buffer, 0, 400000);
    bytes = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes < 0)
        throw std::runtime_error("Core.cpp:110\n");
    else if (bytes == 0) // Client closed connetiion
        close_client(client_fd);
    else if (bytes > 0)
    {
        Client &client = this->_clients[client_fd];

        client.set_resquest(HttpRequest(buffer));
        std::cout << "[ Request ] \n";
        std::cout << client.get_request() << "\n";

        client.set_response(HttpResponse(client.get_request()));
        std::cout << "[ Response ] \n";
        std::cout << client.get_response() << "\n";

        // Switch to write mode
        FD_SET(client_fd, &this->_write_set); // Add to write set
        client.set_state(WRITING_HEADER);
        client.set_last_activity();
    } 
}

void Core::handle_write(int client_fd)
{
    Client              &client     = this->_clients[client_fd];
    HttpResponse        &response   = client.get_response();
    ssize_t             bytes_sent;
    ssize_t             bytes;

    switch (client.get_state()) {
        case WRITING_HEADER:
            std::cout << " [ Sending Header ] " << "\n";
            bytes = send(client_fd, &response.get_header()[0], response.get_header().length(), 0);
            if (bytes == -1)
                throw std::runtime_error("Core.cpp:146\n");
            client.set_state(WRITING_BODY);
            break ;

        case WRITING_BODY:
            std::cout << " [ Sending Body ] " << "\n";
            bytes_sent = client.get_bytes_sent();
            bytes = send(client_fd, &response.get_body()[bytes_sent], response.get_body().size() - bytes_sent, 0);
            if (bytes == -1)
                throw std::runtime_error("Core.cpp:156\n");
            else if (bytes == 0) // Response all sent
            {
                std::cout << "[ Response All Sent ] " << "\n";
                //FD_CLR(client_fd, &this->_write_set); // Remove from writting set
                if (response.get_connection().compare("close") == 0)
                    this->close_client(client_fd);
                client.set_bytes_sent(0);
                client.set_state(READING);
            }
            bytes_sent += bytes;
            client.set_bytes_sent(bytes_sent);
            break ;

        default:
            break;
    }

    client.set_last_activity();
}

void Core::close_client(int fd)
{
    close(fd);
    FD_CLR(fd, &this->_read_set);
    FD_CLR(fd, &this->_write_set);
    this->_clients.erase(fd);
    std::cout << "Connetion Closed with client: " << fd << "\n";
}


// Check the last activity from the client and close the connection if a timeout has exceeded
void Core::check_timeouts()
{
    time_t              now;
    std::vector<int>    to_remove;
    
    now = time(0);
    for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
    {
        if (now - it->second.get_last_activity() > this->_timeout)
            to_remove.push_back(it->first);
    }

    for (std::vector<int>::iterator it = to_remove.begin(); it != to_remove.end(); ++it)
        close_client(*it);
}


