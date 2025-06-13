#include "../includes/Core.hpp"

Core::Core()
{
    throw std::logic_error("Provid port to listen on: Core(int port)");
}

// Inicialize the servers by adding to the servers map
// Each server is accessible by its file descriptor
Core::Core(std::vector<ServerBlock> server_blocks)
:_timeout(75),_client_connection(false) // Connetion timeout 75 seconds
{   
    this->_biggest_fd = 2;
    FD_ZERO(&this->_read_set);
    FD_ZERO(&this->_write_set);

    for (std::vector<ServerBlock>::iterator it = server_blocks.begin(); it != server_blocks.end(); ++it)
    {
        Log::server_start(it->fd, std::atoi(it->listen.c_str()));
        this->_servers.insert(std::make_pair(it->fd, *it));
        FD_SET(it->fd, &this->_read_set); // Add server fd to the read set
        if (it->fd > this->_biggest_fd)
            this->_biggest_fd = it->fd;
    }
}

/* void Core::cleanup() {
    this->_clients.clear();
} */

// Accepts client connection to the given server
void Core::get_client(int server_fd)
{
    socklen_t                            addr_len;
    int                                  client_fd;
    std::map<int, ServerBlock>::iterator serv_it;

    addr_len = sizeof(struct sockaddr_in);
    serv_it = this->_servers.find(server_fd);
    if (serv_it != this->_servers.end())
    {
        client_fd = accept(server_fd, (struct sockaddr *)&(serv_it->second.addr), &addr_len);
        if (client_fd == -1)
        	throw std::runtime_error("Core.cpp:39");
        else if (client_fd >= 0)
        {
            this->_clients.insert(std::make_pair(client_fd, Client(client_fd, server_fd)));
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
void Core::client_multiplex(char **env)
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
        for (std::map<int, ServerBlock>::iterator it = this->_servers.begin(); it != this->_servers.end(); ++it)
        {
            if (FD_ISSET(it->first, &read_set_copy)) // Check if fd is still present in the set
                this->get_client(it->first); // Accept a client for the serve
        }

        // Check clients
        for (std::map<int, Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); ++it)
        {
            if (FD_ISSET(it->first, &read_set_copy) || // checks if client fd is in set 
                FD_ISSET(it->second.get_file_fd(), &read_set_copy) || // Checks if cliet file fd is in set
                FD_ISSET(it->second.get_cgi_stdin(), &write_set_copy) ||  // Check if cgi stdin is ready to write to
                FD_ISSET(it->second.get_cgi_stdout(), &read_set_copy)) // Check if cgi stdout is ready to read from
            {
                switch (it->second.get_client_state())
                {
                    case BUILD_REQUEST:
                        this->build_request(it->first, env); // Call recv()
                        break ;
                    case CGI_PROCESSING:
                        this->handle_cgi_output(it->second);
                        break;
                    case BUILD_RESPONSE:
                        this->build_response(it->first); // read from file
                        break;
                    default:
                        break;
                }
            }
            else if (it->second.get_client_state() == BUILD_RESPONSE_FROM_CGI) // this branch is needed bc we dont open any fd, so we dont enter the previous branch
                this->build_response(it->first);
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

void Core::build_request(int client_fd, char **env)
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

        client.set_last_activity();
        Log::building_request(client_fd);
        client.set_resquest(buffer, bytes, this->_servers[client.get_server_fd()]); // Request Parser
        if (client.get_parser_state() == BODY) // body not all parsed
        {
            client.set_client_state(BUILD_REQUEST);
            return ;
        }
        if (client.get_is_cgi() == true) // Request to CGI
            this->execute_cgi(client, env);
        else
        {
            client.set_client_state(BUILD_RESPONSE);
            client.set_file(client.get_path().c_str()); // For response body, eg. index.html
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
}

void Core::build_response(int client_fd)
{
    Client &client = this->_clients[client_fd];

    Log::building_response(client_fd);
    client.set_response(); // Reads and close the file descriptor if was not empty
    if (client.get_content_lenght() != 0 && client.get_client_state() != BUILD_RESPONSE_FROM_CGI)
    {
        FD_CLR(client.get_file_fd(), &this->_read_set); // rm file from read set
        Log::rm_from_read(client.get_file_fd());
        if (client.get_file_fd() == this->_biggest_fd)
            --this->_biggest_fd; // remove file fd 
        Log::close_file(client.get_file_fd());
        if (client.get_file_fd() != -1)
        {
            if (close(client.get_file_fd()) == -1)
                throw std::runtime_error("Core.cpp:184");
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

void    Core::execute_cgi(Client &client, char **env)
{
    int stdin_pipe[2];
    int stdout_pipe[2];
    pid_t pid;

    // Create pipes for CGI communication
    if (pipe(stdin_pipe) < 0 || pipe(stdout_pipe) < 0)
        throw std::runtime_error("Pipe creation failed");

    pid = fork();
    if (pid == -1)
        throw std::runtime_error("Fork failed");
    else if (pid == 0)  // Child process (CGI)
    {
        close(stdin_pipe[1]);  // Close write end of stdin pipe
        close(stdout_pipe[0]); // Close read end of stdout pipe

        // Redirect standard streams
        dup2(stdin_pipe[0], STDIN_FILENO);
        dup2(stdout_pipe[1], STDOUT_FILENO);

        // Close unused pipe ends
        close(stdin_pipe[0]);
        close(stdout_pipe[1]);

        // TODO: Prepare CGI environment
        char** cgi_envp = env;

        // Execute CGI
        std::string script_path = "./cgi-bin/" + client.get_path();
        const char* argv[] = {script_path.c_str(), NULL};

        execve(script_path.c_str(), const_cast<char**>(argv), cgi_envp);

        //TODO: Cleanup if execve fails
        exit(EXIT_FAILURE);
    }
    // Close unused pipe ends
    close(stdin_pipe[0]);
    close(stdout_pipe[1]);

    // Set pipes to not block
    fcntl(stdout_pipe[0], F_SETFL, O_NONBLOCK);
    fcntl(stdin_pipe[1], F_SETFL, O_NONBLOCK);

    // Store CGI information in client
    client.set_cgi_pid(pid);
    client.set_cgi_stdin(stdin_pipe[1]);   // Write to CGI's stdin
    client.set_cgi_stdout(stdout_pipe[0]);  // Read from CGI's stdout
    client.set_client_state(CGI_PROCESSING);

    // Register pipes with select()
    //FD_SET(stdin_pipe[1], &this->_write_set);
    FD_SET(stdout_pipe[0], &this->_read_set);

    // Update biggest_fd if needed
    //if (stdin_pipe[1] > this->_biggest_fd)
    //    this->_biggest_fd = stdin_pipe[1];
    if (stdout_pipe[0] > this->_biggest_fd)
        this->_biggest_fd = stdout_pipe[0];

    FD_CLR(client.get_fd(), &this->_read_set); // rm from read set
    if (client.get_fd() == this->_biggest_fd)
        --this->_biggest_fd;

    // TODO: Write POST data to CGI (if any)
    // if (!client.get_body().empty()) {
    //     client.set_cgi_input(client.get_body());
    // }
    // is Post sent via request body or PATH_INFO?
}

void    Core::handle_cgi_output(Client &client)
{
    char    buffer[BUFFER_SIZE];
    ssize_t bytes;
    std::vector<char> &cgi_output = client.get_cgi_output();


    memset(buffer, 0, BUFFER_SIZE);
    bytes = read(client.get_cgi_stdout(), buffer, BUFFER_SIZE - 1);
    if (bytes == -1)
        throw std::runtime_error("Core.cpp:383");
    if (bytes == 0) // CGI send EOF
    {
        client.set_client_state(BUILD_RESPONSE_FROM_CGI);

        FD_CLR(client.get_cgi_stdout(), &this->_read_set); // rm from read set
        if (client.get_fd() == this->_biggest_fd)
            --this->_biggest_fd;
        close(client.get_cgi_stdout());
        client.set_cgi_stdout(-1);
        return ;
    }
    else if (bytes > 0)
        cgi_output.insert(cgi_output.end(), buffer, buffer + bytes);
}

void    Core::check_cgi_output(Client &client)
{
    std::vector<char> &cgi_output = client.get_cgi_output();

    for (std::vector<char>::iterator it = cgi_output.begin(); it != cgi_output.end(); ++it)
        std::cout << *it;
    std::cout << "\n";
    exit(1);
}
