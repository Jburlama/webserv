#include "../includes/HttpResponse.hpp"

HttpResponse::HttpResponse(HttpRequest &request, int clientfd)
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
            this->set_body("content/html/index.html");
            this->set_str();
        }
        else if (this->get_status_code() == 404)
        {
            this->set_body("html/error_pages/page_not_found.html");
            this->set_str();
        }
    }
    else if (request.get_method().compare("POST") == 0)
    {
        std::cout << "Content Type: " << headers["Content-Type"][0] << "\n";
        std::cout << request;

        // TODO: multipart/form-data
        if (headers["Content-Type"][0].compare("multipart/form-data") == 0)
        {
            std::cout << "Content-Type is multipart/form-data\n";
        }
        else if (headers["Content-Type"][0].compare("application/octet-stream") == 0)
        {
            std::cout << request;
        }
        this->set_str();
    }
    if (send(clientfd, this->get_str().c_str(), strlen(this->get_str().c_str()) + 1, 0) == -1)
        throw std::runtime_error("HttpResponse.cpp:line:18\n");
}

std::string HttpResponse::get_str()
{
    return this->_str;
}

std::string HttpResponse::get_version()
{
    return this->_version;
}
int HttpResponse::get_status_code()
{
    return this->_status_code;
}

std::string HttpResponse::get_descrition()
{
    return this->_description;
}

std::string HttpResponse::get_date()
{
    return this->_date;
}

size_t HttpResponse::get_content_lenght()
{
    return this->_content_length;
}

std::vector<std::string> HttpResponse::get_content_type()
{
    return this->_content_type;
}

std::string HttpResponse::get_server()
{
    return this->_server;
}

std::string HttpResponse::get_body()
{
    return this->_body;
}

void HttpResponse::set_str()
{
    std::ostringstream stream;

    this->_str = this->_version + " ";
    switch (this->_status_code)
    {
        case 200:
            this->_str += "200 " + this->_description + "\r\n";
            break ;
        case 404:
            this->_str += "404 " + this->_description + "\r\n";
        default:
            break;
    }
    if (!this->_date.empty())
        this->_str += "Date: " + this->_date + "\r\n";
    if (!this->_content_type.empty())
    {
        for (std::vector<std::string>::iterator it = this->_content_type.begin(); it != this->_content_type.end(); ++it)
            this->_str += "Content-Type: " + *it + "\r\n";
    }
    if (!this->_server.empty())
        this->_str += "Server: " + this->_server + "\r\n";
    if (this->_content_length != 0)
    {
        stream << this->_content_length;
        this->_str += "Content-Length: " + stream.str() + "\r\n";
    }
    this->_str += "\r\n";
    if (!this->_body.empty())
        this->_str += this->_body;
}

void HttpResponse::set_version(std::string version)
{
    this->_version = version;
}

void HttpResponse::set_status_code(int code)
{
    this->_status_code = code;
    switch (code)
    {
        case 200:
            this->_description = "OK";
        case 404:
            this->_description = "Not Found";
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

void HttpResponse::set_content_type(std::string type)
{
    this->_content_type.insert(this->_content_type.end(), type);
}

void HttpResponse::set_server(std::string name)
{
    this->_server = name;
}

void HttpResponse::set_body(std::string file_path)
{
    std::ifstream   file(file_path.c_str());
    std::string     line;

    if (!file.is_open())
        throw std::runtime_error("Failed to open file\n");
    while (std::getline(file, line))
        this->_body += line + '\n';
    file.close();
    this->_content_length = strlen(this->_body.c_str());
}
