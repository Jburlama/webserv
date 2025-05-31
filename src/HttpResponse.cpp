#include "../includes/HttpResponse.hpp"

HttpResponse::HttpResponse(HttpRequest &request)
{
    std::map<std::string, std::vector<std::string> > headers;
    headers = request.get_headers();

    this->set_version("HTTP/1.1");
    this->set_server("webserv");
    this->set_date();
    this->_content_length = 0;
    if (request.get_method().compare("GET") == 0)
    {
        this->set_content_type("text/html");
        this->set_status_code(200);
        if (this->get_status_code() == 200)
        {
            this->set_header();
            this->set_body_from_file("content/html/index.html");
        }
        else if (this->get_status_code() == 404)
        {
            this->set_header();
            this->set_body_from_file("html/error_pages/page_not_found.html");
        }
    }
    else if (request.get_method().compare("POST") == 0)
    {
        this->set_body(request.get_body());
        this->set_content_type("text/html");
        this->set_status_code(200);
        if (this->get_status_code() == 200)
        {
            this->set_header();
            this->set_body_from_file("content/html/index.html");
        }
    }
}

void HttpResponse::set_header()
{
    std::ostringstream stream;

    this->_header = this->_version + " ";
    switch (this->_status_code)
    {
        case 200:
            this->_header += "200 " + this->_description + "\r\n";
            break ;
        case 404:
            this->_header += "404 " + this->_description + "\r\n";
        default:
            break;
    }
    if (!this->_date.empty())
        this->_header += "Date: " + this->_date + "\r\n";
    if (!this->_content_type.empty())
    {
        for (std::vector<std::string>::iterator it = this->_content_type.begin(); it != this->_content_type.end(); ++it)
            this->_header += "Content-Type: " + *it + "\r\n";
    }
    if (!this->_server.empty())
        this->_header += "Server: " + this->_server + "\r\n";
    if (this->_content_length != 0)
    {
        stream << this->_content_length;
        this->_header += "Content-Length: " + stream.str() + "\r\n";
    }
    this->_header += "\r\n";
}

void HttpResponse::set_status_code(int code)
{
    this->_status_code = code;
    switch (code)
    {
        case 200:
            this->_description = "OK";
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


void HttpResponse::set_body(const std::vector<char> &body)
{
    this->_body = body;
    this->_content_length = body.size();
}

void HttpResponse::set_body_from_file(const std::string &file_path)
{
    std::ifstream file(file_path.c_str(), std::ios::binary);
    if (!file)
        throw std::runtime_error("HttpResponse:157");
    
    file.seekg(0, std::ios::end);
    this->_content_length = file.tellg();
    file.seekg(0, std::ios::beg);
    
    this->_body.resize(_content_length);
    file.read(&this->_body[0], this->_content_length);
    file.close();
}
