#include "../includes/Core.hpp"
#include <sys/select.h>

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
        if (ready == -1)
            throw std::runtime_error("Core.cpp:66\n");
        
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
            if (FD_ISSET(it->first, &this->_read_set) || // checks if client fd is in set 
                FD_ISSET(it->second.get_file_fd(), &this->_read_set)) // Checks if cliet file fd is in set
            {
                switch (it->second.get_client_state())
                {
                    case BUILD_REQUEST:
                        std::cout << "Reading from client: " << it->first << "\n";
                        this->build_request(it->first); // Call recv()
                        std::cout << "Request build it for client: " << it->first << "\n";
                        break ;
                    case BUILD_RESPONSE:
                        std::cout << "Building Response for client: " << it->first << "\n";
                        this->build_response(it->first);
                        break;
                    default:
                        break;
                }
            }
            else if (FD_ISSET(it->first, &this->_write_set)) // Check if client is in write set
            {
                std::cout << "Writing to Client: " << it->first << "\n";
                this->handle_write(it->first); // Call send().
            }
        }
        // Close a client connection if timeout as exceeded
        this->check_timeouts();
    }
}

void Core::build_request(int client_fd)
{                 
    char    buffer[BUFFER_SIZE];
    ssize_t bytes;

    memset(buffer, 0, BUFFER_SIZE);
    bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, 0);
    if (bytes < 0)
        throw std::runtime_error("Core.cpp:110\n");
    else if (bytes == 0) // Client closed connetiion
        close_client(client_fd);
    else if (bytes > 0)
    {
        Client  &client = this->_clients[client_fd];
        int     file_fd;

        std::cout << "Building Request for " << client_fd << "\n";
        client.set_resquest(buffer, bytes);
        file_fd = client.get_file_fd();
        FD_SET(file_fd, &this->_read_set); // add the client fd to read set
        client.set_last_activity();
    } 
}

void Core::build_response(int client_fd)
{
    Client &client = this->_clients[client_fd];

    client.set_response();
    client.set_client_state(WRITING_HEADER);
    client.set_last_activity();
    FD_CLR(client.get_file_fd(), &this->_read_set); // rm file from read set
    FD_CLR(client_fd, &this->_read_set); // rm from read set
    FD_SET(client_fd, &this->_write_set); // Add to write set
    return ;
}

void Core::handle_write(int client_fd)
{
    Client              &client     = this->_clients[client_fd];
    ssize_t             bytes_sent;
    ssize_t             bytes;


    switch (client.get_client_state()) {
        case WRITING_HEADER:
            std::cout << " [ Sending Header ] " << "\n";
            bytes = send(client_fd, &client.get_response_header()[0], client.get_response_header().length(), 0);
            if (bytes == -1)
                throw std::runtime_error("Core.cpp:160\n");
            client.set_client_state(WRITING_BODY);
            break ;

        case WRITING_BODY:
            std::cout << " [ Sending Body ] " << "\n";
            bytes_sent = client.get_bytes_sent();
            bytes = send(client_fd, client.get_response_body() + bytes_sent, client.get_content_lenght() - bytes_sent, 0);
            if (bytes == -1)
                throw std::runtime_error("Core.cpp:169\n");
            else if (bytes > 0)
            {
                bytes_sent = bytes + client.get_bytes_sent();
                if (bytes_sent == client.get_content_lenght()) // Body all sent
                {
                    std::cout << "[ Response All Sent ] " << "\n";
                    if (client.get_connection().compare("close") == 0)
                        this->close_client(client_fd);
                    delete [] client.get_response_body();
                    client.set_bytes_sent(0);
                    client.set_client_state(BUILD_REQUEST);
                    FD_CLR(client_fd, &this->_write_set);
                    FD_SET(client_fd, &this->_read_set);
                }
            }
            bytes_sent += bytes;
            client.set_bytes_sent(bytes_sent);
            break ;

        default:
            break;
    }
    client.set_last_activity();
}

// TODO: Remember to Close file fd
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
    
    now = time(0);
    for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
    {
        if (now - it->second.get_last_activity() > this->_timeout)
        {
            std::cout << "[ Timeout ] closing: " << it->first << "\n";
            close_client(it->first);
        }
    }
}
