#include "../includes/HttpResponse.hpp"
#include <sys/types.h>

HttpResponse::HttpResponse()
: _version("HTTP/1.1"), _status_code(200), _description("OK"),
_content_length(0), _server("webserv"), _response_body(NULL)
{
    set_date();
}

HttpResponse &HttpResponse::operator=(HttpResponse &other)
{
    this->_response_header  = other.get_response_header();
    this->_version          = other.get_version();
    this->_status_code      = other.get_status_code();
    this->_description      = other.get_descrition();
    this->_date             = other.get_date();
    this->_content_type     = other.get_content_type();
    this->_content_length   = other.get_content_lenght();
    this->_server           = other.get_server();
    this->_response_body    = other.get_response_body();
    this->_connection       = other.get_connection();
    this->_response_body    = other.get_response_body();

    return *this;
}


// NOTE: Function beeing rebuild in Client::set_request()
HttpResponse::HttpResponse(HttpRequest &request)
{
    std::map<std::string, std::vector<std::string> > headers;
    headers = request.get_request_headers();

    this->set_version("HTTP/1.1");
    this->set_server("webserv");
    if (headers.find("Connection") != headers.end())
        this->set_connection(headers["Connection"][0]);
    this->set_date();
    this->_content_length = 0;
    this->set_content_type("text/html");
    if (request.get_method().compare("GET") == 0)
    {
        this->set_status_code(200);
        this->set_response_header();
    }
    else if (request.get_method().compare("POST") == 0)
    {
        this->set_status_code(201);
        if (this->get_status_code() == 200)
            this->set_response_header();
        if (this->get_status_code() == 201)
            this->set_response_header();
    }
}

void HttpResponse::set_response_header()
{
    std::ostringstream stream;

    this->_response_header = this->_version + " ";
    switch (this->_status_code)
    {
        case 200:
            this->_response_header += "200 " + this->_description + "\r\n";
            break ;
        case 201:
            this->_response_header += "201 " + this->_description + "\r\n";
            break ;
        case 204:
            this->_response_header += "204 " + this->_description + "\r\n";
            break ;
        case 404:
            this->_response_header += "404 " + this->_description + "\r\n";
        default:
            break;
    }
    if (!this->_date.empty())
        this->_response_header += "Date: " + this->_date + "\r\n";
    if (!this->_content_type.empty())
    {
        for (std::vector<std::string>::iterator it = this->_content_type.begin(); it != this->_content_type.end(); ++it)
            this->_response_header += "Content-Type: " + *it + "\r\n";
    }
    if (!this->_server.empty())
        this->_response_header += "Server: " + this->_server + "\r\n";
    if (this->_content_length != 0)
    {
        stream << this->_content_length;
        this->_response_header += "Content-Length: " + stream.str() + "\r\n";
    }
    this->_response_header += "Connection: " + this->_connection + "\r\n";
    this->_response_header += "\r\n";
}

void HttpResponse::set_status_code(int code)
{
    this->_status_code = code;
    switch (code)
    {
        case 200:
            this->_description = "OK";
            break ;
        case 201:
            this->_description = "Created";
            break ;
        case 204:
            this->_description = "No Content";
            break ;
        case 404:
            this->_description = "Not Found";
            break ;
        default:
            break;
    }
}

void HttpResponse::set_date()
{
    char date[1000];
    time_t now = time(0);

    memset(date, 0, 1000);
    struct tm tm = *gmtime(&now);
    strftime(date, sizeof(date), "%a, %d %b %Y %H:%M:%S %Z", &tm);

    this->_date = date;
}


std::ostream &operator<<(std::ostream &os, HttpResponse &response)
{
    os << "Status Code: " << response.get_status_code() << "\n";
    os << "Description: " << response.get_descrition() << "\n";
    os << "Date: " << response.get_date() << "\n";
    os << "Content lenght: " << response.get_content_lenght() << "\n";
    os << "Content type: " << response.get_content_type()[0] << "\n";
    os << "Connection: " << response.get_connection() << "\n";
    os << "Body: " << std::string(&response.get_response_body()[0]) << "\n";

    return os;
}
