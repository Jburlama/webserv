#include "../includes/Core.hpp"
#include <cstdio>

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
            this->_clients.insert(std::make_pair(client_fd, Client(client_fd)));
            FD_SET(client_fd, &this->_read_set); // add the client to read set
        }
    }
}

void Core::handle_read(int client_fd)
{                 
    char    buffer[4000000];
    ssize_t bytes;

    bytes = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes > 0)
    {
        std::cout << "Bytes: " << bytes << "\n";
        Client &client = this->_clients[client_fd];
        std::vector<char> &read_buf = client.get_read_buffer();

        read_buf.insert(read_buf.end(), buffer, buffer + bytes);
        client.set_last_activity();
        std::cout << std::string(&read_buf[0]);
        try
        {
            HttpRequest     request(read_buf); // Parse Request
            read_buf.clear();
            HttpResponse    response(request); // Build Response

            client.set_response(response);

            // Switch to write mode
            FD_CLR(client_fd, &this->_read_set); // Remove from read set
            FD_SET(client_fd, &this->_write_set); // Add to write set
        }
        catch (const std::exception& e) {
            std::cout << e.what() << "\n";
            throw std::logic_error("Http request parser fails");
        }
    } 
    else if (bytes <= 0)
        close_client(client_fd);
}

void Core::handle_write(int client_fd)
{
    Client              &client     = this->_clients[client_fd];
    HttpResponse        &response   = client.get_response();
    ssize_t             bytes;

    if (client.get_header_sent()) // Sends the header first
    {
        bytes = send(client_fd, &response.get_header()[0], response.get_header().length(), 0);
        client.set_header_sent(false);
    }
    else
    {
        bytes = send(client_fd, &response.get_body()[0], response.get_body().size(), 0); // sends the body
        if ((client.get_bytes_sent() + bytes) < response.get_body().size()) // Check if the body was all sent
            client.set_bytes_sent(client.get_bytes_sent() + bytes);
        else // Close connection if the body was all sent
        {
            FD_CLR(client_fd, &this->_write_set);
            close(client_fd);
            client.set_header_sent(true);
            response.get_body().clear();
            client.set_bytes_sent(0);
        }
    }
    client.set_last_activity();
}

void Core::close_client(int fd)
{
    close(fd);
    FD_CLR(fd, &this->_read_set);
    FD_CLR(fd, &this->_write_set);
    this->_clients.erase(fd);
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


// Handle multiple clients
void Core::client_multiplex()
{
    fd_set          read_ready;
    fd_set          write_ready;
    struct timeval  tv = {1, 0}; // 1 sec timeout, so that select doesn't block;
    int             ready;
    
    while (42)
    {
        // because select is destructive and will change the set
        read_ready  = this->_read_set;
        write_ready = this->_write_set;
        
        ready = select(FD_SETSIZE, &read_ready, &write_ready, NULL, &tv);
        if (ready < 0)
            throw std::runtime_error("Core.cpp:154\n");

        // Check servers
        for (std::map<int, Server>::iterator it = this->_servers.begin(); it != this->_servers.end(); ++it)
        {
            if (FD_ISSET(it->first, &read_ready)) // Check if fd is still present in the set
                this->get_client(it->first); // Accept a client for the server
        }

        // Check clients
        for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
        {
            int fd = it->first;
            if (FD_ISSET(fd, &this->_read_set)) // Check if client is in read set
                this->handle_read(fd);
            else if (FD_ISSET(fd, &this->_write_set)) // Check if client is in write set
                this->handle_write(fd);
        }
        // Close a client connection if timeout as exceeded
        this->check_timeouts();
    }
}
