#include "../includes/HttpResponse.hpp"

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

std::map<std::string, std::vector<std::string> > HttpResponse::get_headers()
{
    return this->_headers;
}

std::string HttpResponse::get_body()
{
    return this->_body;
}

void HttpResponse::set_str(std::string str)
{
    this->_str = str;
}

void HttpResponse::set_version(std::string version)
{
    this->_version = version;
}

void HttpResponse::set_status_code(int code)
{
    this->_status_code = code;
}

void HttpResponse::set_descrition(std::string description)
{
    this->_description = description;
}

void HttpResponse::set_headers(std::map<std::string, std::vector<std::string> > headers)
{
    this->_headers = headers;
}

void HttpResponse::set_body(std::string body)
{
    this->_body = body;
}
