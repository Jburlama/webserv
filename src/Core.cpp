#include "../includes/Core.hpp"

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
    char buffer[4096];
    ssize_t bytes;

    bytes = recv(client_fd, buffer, sizeof(buffer), 0);
    if (bytes > 0)
    {
        Client &client = this->_clients[client_fd];
        std::vector<char> &read_buf = client.get_read_buffer();

        read_buf.insert(read_buf.end(), buffer, buffer + bytes);
        client.set_last_activity();

        try
        {
            // Try to parse the request
            HttpRequest request(read_buf);
            
            // Request is complete, create response
            HttpResponse response(request);
            
            // Store response in client
            client.get_write_buffer() = response.set_body_from_file("content/html/index.html");
            client.set_response_ready(true);
            
            // Switch to write mode
            FD_CLR(client_fd, &_read_set);
            FD_SET(client_fd, &_write_set);
        }
        catch (const std::exception& e) {
            // Parsing failed (incomplete request)
            // Wait for more data
        }
    } 
    else if (bytes <= 0)
        close_client(client_fd);
}

void Core::handle_write(int client_fd)
{
    Client              &client     = this->_clients[client_fd];
    std::vector<char>   &write_buf  = client.get_write_buffer();
    size_t              &offset     = client.get_write_offset();
    ssize_t             bytes;

    bytes = send(client_fd, &write_buf[offset], write_buf.size() - offset, 0);

    if (bytes > 0)
    {
        offset += bytes;
        client.set_last_activity();

        if (offset >= write_buf.size())
        {
            // Request/response cycle complete
            if (/* keep-alive requested */)
            {
                // Reset client for new request
                client.get_read_buffer().clear();
                client.get_response_data().clear();
                client.set_response_ready(false);
                offset = 0;
                FD_CLR(client_fd, &_write_set);
                FD_SET(client_fd, &_read_set);
            }
            else
                close_client(client_fd);
        }
    } 
    else if (bytes <= 0)
        close_client(client_fd);
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
    struct timeval  tv;
    int             ready;

    tv = {1, 0}; // 1 sec timeout, so that select doesn't block
    
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
            if (FD_ISSET(fd, &read_ready)) // Check if client is in read set
                this->handle_read(fd);
            else if (FD_ISSET(fd, &write_ready)) // Check if client is in write set
                this->handle_write(fd);
        }
        // Close a client connection if timeout as exceeded
        this->check_timeouts();
    }
}
