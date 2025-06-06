#include "../includes/Core.hpp"

Core::Core()
{
    throw std::logic_error("Provid port to listen on: Core(int port)");
}

// Inicialize the servers by adding to the servers map
// Each server is accessible by its file descriptor
Core::Core(std::vector<int> ports)
:_timeout(75),_client_connection(false) // Connetion timeout 75 seconds
{   
    this->_biggest_fd = 2;
    FD_ZERO(&this->_read_set);
    FD_ZERO(&this->_write_set);
    for (std::vector<int>::iterator it = ports.begin(); it != ports.end(); ++it)
    {
        Server server(*it);

        Log::server_start(server.get_fd(), *it);
        this->_servers.insert(std::make_pair(server.get_fd(), server));
        FD_SET(server.get_fd(), &this->_read_set); // Add server fd to the read set
        if (server.get_fd() > this->_biggest_fd)
            this->_biggest_fd = server.get_fd();
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
        	throw std::runtime_error("Core.cpp:39");
        else if (client_fd >= 0)
        {
            this->_clients.insert(std::make_pair(client_fd, Client(client_fd)));
            Log::server_accept_client(server_fd, client_fd);
            FD_SET(client_fd, &this->_read_set); // add the client to read set
            if (client_fd > this->_biggest_fd)
                this->_biggest_fd = client_fd;
            Log::on_read(client_fd);
        }
    }
}

// Handle multiple clients
// Main loop of the program
void Core::client_multiplex()
{
    fd_set          read_set_copy;
    fd_set          write_set_copy;
    struct timeval  tv;

    while (42)
    {
        //  sec timeout, so that select doesn't block;
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        // Copy because select is destructive and will change the sets
        read_set_copy  = this->_read_set;
        write_set_copy = this->_write_set;

        if (select(this->_biggest_fd + 1, &read_set_copy, &write_set_copy, NULL, &tv) == -1)
            throw std::runtime_error("Core.cpp:71");
        
        // check servers
        for (std::map<int, Server>::iterator it = this->_servers.begin(); it != this->_servers.end(); ++it)
        {
            if (FD_ISSET(it->first, &read_set_copy)) // Check if fd is still present in the set
                this->get_client(it->first); // Accept a client for the serve
        }

        // Check clients
        for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
        {
            if (FD_ISSET(it->first, &read_set_copy) || // checks if client fd is in set 
                FD_ISSET(it->second.get_file_fd(), &read_set_copy)) // Checks if cliet file fd is in set
            {
                switch (it->second.get_client_state())
                {
                    case BUILD_REQUEST:
                        this->build_request(it->first); // Call recv()
                        break ;
                    case BUILD_RESPONSE:
                        this->build_response(it->first); // read from file
                        break;
                    default:
                        break;
                }
            }
            if (this->get_connection_state()) // Without this check the next if would segfault when client closed connection bc the Client was erased
            {
                this->set_connection_state(false);
                break ;
            }
            if (FD_ISSET(it->first, &write_set_copy)) // Check if client is in write set
                this->handle_write(it->first); // Call send().
            if (this->check_timeouts(it->first)) // Close a client connection if timeout as exceeded
                break ;
        }
    }
}

void Core::build_request(int client_fd)
{                 
    char    buffer[BUFFER_SIZE];
    ssize_t bytes;

    memset(buffer, 0, BUFFER_SIZE);
    bytes = recv(client_fd, buffer, BUFFER_SIZE - 1, MSG_DONTWAIT);
    if (bytes == -1)
        return ;
    else if (bytes == 0) // Client closed connetiion
    {
        close_client(client_fd);
        this->set_connection_state(true);
    }
    else if (bytes > 0)
    {
        Client  &client = this->_clients[client_fd];
        int file_fd;

        Log::building_request(client_fd);
        client.set_resquest(buffer, bytes);
        client.set_client_state(BUILD_RESPONSE);
        client.set_last_activity();
        client.set_file(client.get_path().c_str());
        if (client.get_file_bytes() != 0) // File is not empty, if it was fd was alreay closed in set_file()
        {
            // we add to the set here bc the read_set master is in the Core class
            file_fd = client.get_file_fd();
            FD_SET(file_fd, &this->_read_set); // add the file fd to read set
            if (file_fd > this->_biggest_fd)
                this->_biggest_fd = file_fd;
            Log::on_read(file_fd);
        }
        else // File is empty so we build response right away, bc we dont need to read from file
            this->build_response(client_fd);
        FD_CLR(client_fd, &this->_read_set);
    } 
}

void Core::build_response(int client_fd)
{
    Client &client = this->_clients[client_fd];

    Log::building_response(client_fd);
    client.set_response(); // Reads and close the file descriptor if was not empty
    if (client.get_content_lenght() != 0)
    {
        FD_CLR(client.get_file_fd(), &this->_read_set); // rm file from read set
        Log::rm_from_read(client.get_file_fd());
        if (client.get_file_fd() == this->_biggest_fd)
            --this->_biggest_fd; // remove file fd 
        Log::close_file(client.get_file_fd());
        if (client.get_file_fd() != -1)
        {
            if (close(client.get_file_fd()) == -1)
                throw std::runtime_error("Core.cpp:169");
        }
        client.set_closed_file_fd(); // Set file fd to -1
    }
    client.set_client_state(WRITING_HEADER); 
    client.set_last_activity();
    FD_CLR(client_fd, &this->_read_set); // rm from read set
    Log::rm_from_read(client_fd);
    FD_SET(client_fd, &this->_write_set); // Add to write set
    Log::on_write(client_fd);
    return ;
}

void Core::handle_write(int client_fd)
{
    Client              &client     = this->_clients[client_fd];
    ssize_t             bytes_sent;
    ssize_t             bytes;


    switch (client.get_client_state()) {
        case WRITING_HEADER: // Assumes a small header so will send all in one go
            Log::sending_header(client_fd);
            bytes = send(client_fd, &client.get_response_header()[0], client.get_response_header().length(), 0);
            if (bytes == -1)
                return ;
            client.set_client_state(WRITING_BODY);
            break ;

        case WRITING_BODY:
            if (client.get_content_lenght() == 0) // Empty body
            {
                Log::sent_with_no_body(client_fd);
                if (client.get_connection().compare("close") == 0)
                    this->close_client(client_fd);
                delete [] client.get_response_body();
                client.set_bytes_sent(0);
                client.set_client_state(BUILD_REQUEST);
                FD_CLR(client_fd, &this->_write_set);
                Log::rm_from_write(client_fd);
                FD_SET(client_fd, &this->_read_set);
                Log::on_read(client_fd);
                break ;
            }
            Log::sending_body(client_fd);
            bytes_sent = client.get_bytes_sent();
            bytes = send(client_fd, client.get_response_body() + bytes_sent, client.get_content_lenght() - bytes_sent, 0);
            if (bytes == -1)
                return ;
            else if (bytes > 0)
            {
                bytes_sent = bytes + client.get_bytes_sent();
                if (bytes_sent == client.get_content_lenght()) // Body all sent
                {
                    Log::all_sent(client_fd);
                    if (client.get_connection().compare("close") == 0)
                        this->close_client(client_fd);
                    if (client.get_response_body())
                        delete [] client.get_response_body();
                    client.set_bytes_sent(0);
                    client.set_client_state(BUILD_REQUEST);
                    FD_CLR(client_fd, &this->_write_set);
                    Log::rm_from_write(client_fd);
                    FD_SET(client_fd, &this->_read_set);
                    Log::on_read(client_fd);
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

void Core::close_client(const int fd)
{

    if (FD_ISSET(fd, &this->_read_set))
    {
        FD_CLR(fd, &this->_read_set);
        Log::rm_from_read(fd);
    }
    if (FD_ISSET(fd, &this->_write_set))
    {
        FD_CLR(fd, &this->_write_set);
        Log::rm_from_write(fd);
    }
    Log::connetion_close(fd);
    if (fd != -1)
    {
        if (close(fd) == -1)
            throw std::runtime_error("Core.cpp:250");
    }
    //this->_clients[fd].set_closed_fd();
    this->_clients.erase(fd);
    if (fd == this->_biggest_fd)
        --_biggest_fd;
}


// Check the last activity from the client and close the connection if a timeout has exceeded
bool Core::check_timeouts(int fd)
{
    Client &client = this->_clients[fd];
    
    if (time(NULL) - client.get_last_activity() > this->_timeout)
    {
        Log::timeout(fd);
        close_client(fd);
        return true;
    }
    return false;
}
