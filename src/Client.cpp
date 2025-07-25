#include "../includes/Client.hpp"

Client::Client(int fd, int server_fd)
:_fd(fd),_server_fd(server_fd),_client_state(BUILD_REQUEST),_bytes_sent(0),
_status(0),_is_cgi(false),_cgi_pid(-1),_cgi_stdin(-1),_cgi_stdout(-1),_cgi_bytes_read(-1),_cgi_start_time(0)
{
    this->set_last_activity();
}

Client::~Client()
{
}

void    Client::set_resquest(const char *buffer, ssize_t bytes, ServerBlock &server)
{
    try
    {
        const char *str = &buffer[0];
        std::string url;
        std::string root;
        std::string index;
        std::string temp;
        int         status;

        for (int i = 0; i < bytes + 1; ++i)
        {
            switch (this->_parser_state)
            {
                case START:  // skips empty line, and check if method start at the beguining of the line
                    this->set_has_body(false);
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
                    this->set_is_cgi(false);
                    root = server.root;
                    index = server.index;
                    url = this->_parse_path(i, str);
                    bool has_cgi;

                    has_cgi = false;
                    if (this->_method == "DELETE")
                    {
                        std::string file_to_remove = "upload" + url;
                        std::cout << "File to remove: " << file_to_remove << "\n";
                        std::remove(file_to_remove.c_str());
                    }

                    for (std::vector<LocationBlock>::iterator it = server.locations.begin(); it != server.locations.end(); ++it)
                    {
                        if (!it->cgi_ext.empty())
                            has_cgi = true;
                    }
                    if (url.find(".") == std::string::npos) // url is the location path
                    {
                        for (std::vector<LocationBlock>::iterator it = server.locations.begin(); it != server.locations.end(); ++it)
                        {
                            if (url == it->path || url == "/upload")
                            {
                                if (!it->root.empty())
                                {
                                    if (url != "/upload")
                                        root = it->root + url;
                                    else
                                        root = it->root;
                                }
                                else
                                {
                                    if (url != "/upload")
                                        root += url;
                                }
                                if (!it->index.empty())
                                    index = it->index;
                                break ;
                            }
                            else
                                index = "error_pages/404_not_found.html";
                        }
                        this->_path = root + "/" + index;
                    }
                    else// File is in url
                    {
                        if (has_cgi)
                        {
                            // Check if URL ends with .py (CGI script)
                            if (url.size() > 3 && url.substr(url.size() - 3) == ".py")
                            {
                                this->_path = "cgi-bin/" + url.substr(url.find_last_of('/') + 1);
                                this->set_is_cgi(true);
                            } 
                        }
                        else
                        {
                            std::string::size_type pos = url.find_last_of('/');
                            index = url.substr(pos + 1);
                            if (pos == 0)
                                url = "/";
                            else
                                url = url.substr(0, pos);
                            this->_path = root + "/" + index;
                        }
                    }

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
                    {  
                        this->set_parser_state(START);
                        this->_parser_state = BODY;
                    }
                    else
                        this->_parser_state = END;
                    this->set_bytes_read(i);
                    if (this->get_is_cgi())
                    {
                        this->_status = 302;
                        return ;
                    }
                    break ;

                case BODY:
                    size_t body_size;

                    body_size = 0;
                    if (this->_request_headers.find("Content-Length") != this->_request_headers.end())
                        body_size = std::atoi(this->_request_headers["Content-Length"][0].c_str());
                    else if (this->_request_headers.find("Transfer-Encoding") != this->_request_headers.end())
                        std::cout << "Transfer-Enconding\n" << std::flush;

                    if (body_size > static_cast<size_t>(std::atoi(server.clientMaxBodySize.c_str())))
                        throw std::logic_error("413");

                    while(i < bytes)
                        this->_request_body.insert(this->_request_body.end(), buffer[i++]);


                    this->set_bytes_read(i + this->get_bytes_read());
                    if (this->get_bytes_read() < body_size)
                    {
                        this->_parser_state = BODY;
                        return ;
                    }
                    else
                    {
                        status = this->_parse_request_body();
                        this->set_status(status);
                        this->_parser_state = END;
                        this->set_parser_state(START);
                        if (status == 201)
                            this->set_has_body(true);
                        else if (status == 302)
                            this->set_has_body(false);
                        this->set_upload_length(body_size);
                        this->_request_body.clear();
                        return ;
                    }

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
    catch (const std::logic_error& e)
    {
        std::string error_msg;

        error_msg = e.what();
        if (error_msg.compare("PATH") == 0)
        {
            this->_path = "content/html/error_pages/400_bad_request.html";
            this->_status = 400;
            this->set_parser_state(START);
        }
        else if (error_msg.compare("VERSION") == 0) 
        {
            this->_path = "content/html/error_pages/505_HTTP_Version_Not_Supported.html";
            this->_status = 505;
            this->set_parser_state(START);
        }
        else if (error_msg.compare("413") == 0)
        {
            this->_path = "content/html/error_pages/413_Request_Entity_Too_Large.html";
            this->_status = 413;
            this->set_parser_state(START);
        }
        else
            std::cout << error_msg << "\n" << std::flush;
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
    if (!this->get_location().empty())
        this->set_response_location(this->get_location());
    if (headers.find("Connection") != headers.end())
        this->set_connection(headers["Connection"][0]);
    else
        this->set_connection("keep-alive");
    this->set_response_body(); // Reads and Closes the file fd or build from CGI
    if (this->_content_length != 0)
        this->set_status_code(this->get_status());
    else
        this->set_status_code(204);
    this->set_response_header();
    return ;
}

void Client::set_response_body()
{
    if (this->get_client_state() == BUILD_RESPONSE_FROM_CGI)
    {
        this->_content_length = this->_cgi_output.size();

        if (this->_content_length > 0)
        {
            this->_response_body = new char[this->_content_length];
            if (this->_response_body == NULL)
                throw std::runtime_error("HttpResponse.cpp:168");
            memset(this->_response_body, 0, this->_content_length);

            for (size_t i = 0; i < this->_content_length; ++i)
                this->_response_body[i] = this->_cgi_output[i];
            this->_cgi_output.clear();
        }
    }
    else  // We close the file in Core::build_request, to also remove from fd_set
    {
        this->_content_length = this->_file_stats.st_size;

        if (this->_content_length > 0)
        {
            this->_response_body = new char[this->_content_length];
            if (this->_response_body == NULL)
                throw std::runtime_error("HttpResponse.cpp:214");
            memset(this->_response_body, 0, this->_content_length);
            read(this->_file_fd, this->_response_body, this->_content_length);
        }
    }
}

// Open file for Response body
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

    if (S_ISDIR(this->_file_stats.st_mode))
        std::cout << "This is a directory\n" << std::flush;

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
