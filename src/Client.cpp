#include "../includes/Client.hpp"

Client::Client(int fd)
:_fd(fd),_client_state(BUILD_REQUEST),_bytes_sent(0)
{
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
                    this->_parser_state = VERSION;
                    // TODO: Handdle Errors
                    this->set_file("content/html/index.html"); // Open the fd for the file, dosn't read from it
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
                    this->set_client_state(BUILD_RESPONSE);
                    this->set_parser_state(START);
                    return ;

                default:
                    break;
            }
        }
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << "\n";
        throw std::logic_error("Http request parser fails");
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
    if (this->get_method().compare("GET") == 0)
    {
        this->set_status_code(200);
        this->set_response_body(); // Reads and Closes the file fd
        this->set_response_header();
    }
    return ;
}

void Client::set_response_body()
{
    this->_content_length = this->_file_stats.st_size;

    this->_response_body = new char[this->_content_length];
    if (this->_response_body == NULL)
        throw std::runtime_error("HttpResponse.cpp:167");
    memset(this->_response_body, 0, this->_content_length);

    read(this->_file_fd, this->_response_body, this->_content_length);

    Log::close_file(this->_file_fd);
    close(this->_file_fd);
}

void Client::set_file(const char *file_path)
{
    int file_fd;
    file_fd = open(file_path, O_RDONLY); // Opens the file for read mode
    if (file_fd == -1)
        throw std::runtime_error("Client.cpp:114");
    Log::open_file(file_fd);

    this->_file_fd = file_fd;
    if (this->_file_fd == -1)
        throw std::runtime_error("File.cpp:5");

    if (stat(file_path, &this->_file_stats) == -1) // Gest stats from the file
        throw std::runtime_error("Client.cpp:122");

    this->_file_bytes = this->_file_stats.st_size;
    this->_file_path = file_path;
}
