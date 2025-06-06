#include "../includes/Client.hpp"
#include <sys/select.h>

Client::Client(int fd)
:_fd(fd),_client_state(BUILD_REQUEST),_bytes_sent(0),_status(0)
{
    this->set_last_activity();
}

void    Client::set_resquest(const char *buffer, ssize_t bytes)
{
    try
    {
        const char *str = &buffer[0];

        for (int i = 0; i < bytes + 1; ++i)
        {
            switch (this->_parser_state)
            {
                case START:  // skips empty line, and check if method start at the beguining of the line
                    while (std::isspace(str[i]))
                        ++i;
                    if ((i == 0 && (str[i] == 'G' || str[i] == 'P' || str[i] == 'D'))
                    || (str[i - 1] == '\n' && (str[i] == 'G' || str[i] == 'P' || str[i] == 'D')))
                        this->_parser_state = METHOD;
                    else
                        throw std::logic_error("Error at status line: Method must start at the beguinning of the line");
                    
                case METHOD:
                    this->_method = this->_parse_method(i, str);
                    this->_parser_state = PATH;
                    break;

                case PATH:
                    this->_path = this->_parse_path(i, str);
                    // TODO: Fix when the config file is added
                    this->_path = "content/html/index.html";
                    this->_parser_state = VERSION;
                    break ;

                case VERSION:
                    this->_request_version = this->_parse_request_version(i, str);
                    this->_parser_state = HEADER;
                    break ;

                case HEADER:
                    this->_request_headers = this->_parse_request_header(--i, str);
                    --i;
                    if (this->_request_headers.find("Content-Length") != this->_request_headers.end() ||
                        this->_request_headers.find("Transfer-Encoding") != this->_request_headers.end())
                        this->_parser_state = BODY;
                    else
                        this->_parser_state = END;
                    break ;

                case BODY:
                    // TODO: Parse Body when Client sends a POST
                    // Have to be able to deal with bynayy buffer
                    // the reqeust method has Content-Lenght or Transfer-Encoding
                    this->_request_body = buffer + i;
                    this->_parser_state = END;
                    break ;

                case END:
                    this->_status = 200;
                    this->set_parser_state(START);
                    return ;

                default:
                    break;
            }
        }
    }
    catch (const std::exception& e)
    {
        this->_path = "content/html/error_pages/400_bad_request.html";
        this->_status = 400;
        this->set_parser_state(START);
    }
}

void Client::set_response()
{
    std::map<std::string, std::vector<std::string> > headers;

    headers = this->get_request_headers();

    this->set_bytes_sent(0);
    this->set_version("HTTP/1.1");
    this->set_server("webserv");
    this->set_date();
    if (headers.find("Connection") != headers.end())
        this->set_connection(headers["Connection"][0]);
    else
        this->set_connection("keep-alive");
    if (this->get_method().compare("GET") == 0 || this->get_status() == 400)
    {
        this->set_response_body(); // Reads and Closes the file fd
        if (this->_content_length != 0)
            this->set_status_code(this->get_status());
        else
            this->set_status_code(204);
        this->set_response_header();
    }
    else if (this->get_method().compare("POST") == 0) // Doing the same has GET
    {
        this->set_response_body(); // Reads and Closes the file fd
        if (this->_content_length != 0)
            this->set_status_code(this->get_status());
        else
            this->set_status_code(204);
        this->set_response_header();
    }
    return ;
}

void Client::set_response_body()
{
    this->_content_length = this->_file_stats.st_size;

    if (this->_content_length > 0)
    {
        this->_response_body = new char[this->_content_length];
        if (this->_response_body == NULL)
            throw std::runtime_error("HttpResponse.cpp:167");
        memset(this->_response_body, 0, this->_content_length);
        read(this->_file_fd, this->_response_body, this->_content_length);
        Log::close_file(this->_file_fd);
        close(this->_file_fd); // we remove from fd set in Core::build_response()
    }
}

void Client::set_file(const char *file_path)
{
    int file_fd;
    std::string file;

    file = file_path;
    if (stat(file_path, &this->_file_stats) == -1) // Gest stats from the file
    {
        if (errno == ENOENT) // Set by stat(), if true file dosnt exist 
        {
            file = "content/html/error_pages/404_not_found.html";
            this->set_status(404);
            if (stat(file.c_str(), &this->_file_stats) == -1) // Gest stats from the 404 file
                throw std::runtime_error("Client.cpp:135");
        }
        else
            throw std::runtime_error("Client.cpp:139"); // System fail callign stat()
    }

    if (S_ISDIR(this->_file_stats.st_mode)) // TODO: expand this if file_path is a dir
        std::cout << "This is a directory\n";

    file_fd = open(file.c_str(), O_RDONLY); // Opens the file for read mode
    if (file_fd == -1) // Don't have permition to open file
    {
        file = "content/html/error_pages/403_forbidden.html";
        this->set_status(403);
        if (stat(file.c_str(), &this->_file_stats) == -1) // Gest stats from the 403 file
            throw std::runtime_error("Client.cpp:160");
        file_fd = open(file.c_str(), O_RDONLY); // Opens the the custom 403 file for read mode
        if (file_fd == -1)
            throw std::runtime_error("Client.cpp:162");
    }

    this->_file_fd = file_fd; // the read_set is in Core class so the fd is added to the set in build_response
    Log::open_file(file_fd);

    this->_file_bytes = this->_file_stats.st_size;
    if (this->_file_bytes == 0) // empty file
    {
        Log::close_file(file_fd);
        close(file_fd);
    }

    this->_file_path = file;
}
