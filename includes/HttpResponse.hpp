#pragma once
#include <cstddef>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <fstream>
#include <sys/socket.h>
#include <sstream>
#include <iostream>
#include <stdlib.h>
#include <iostream>

#include "HttpRequest.hpp"

class HttpResponse
{
    private:
        std::string                                         _header; // all the Header
        std::string                                         _version; // HTTP/1.1
        int                                                 _status_code; 
        std::string                                         _description; // status code desc
        std::string                                         _date;
        std::vector<std::string>                            _content_type; // e.g text/html
        size_t                                              _content_length; // the exact byte count of the body
        std::string                                         _server; // software name (webserv)
        std::string                                         _connetion;
        std::vector<char>                                   _body; // page content in binary form
    
    public:
        HttpResponse();
        HttpResponse &operator=(HttpResponse &other);
        HttpResponse(HttpRequest &request);

        std::string                                         get_header()            {return this->_header;};
        std::string                                         get_version()           {return this->_version;};
        int                                                 get_status_code()       {return this->_status_code;};
        std::string                                         get_descrition()        {return this->_description;};
        std::string                                         get_date()              {return this->_date;};
        size_t                                              get_content_lenght()    {return this->_content_length;};
        std::vector<std::string>                            get_content_type()      {return this->_content_type;};
        std::string                                         get_server()            {return this->_server;};
        std::string                                         get_connetion()         {return this->_connetion;};
        std::vector<char>                                   get_body()              {return this->_body;};

        void set_header();
        void set_status_code(int code);
        void set_date();
        void set_body_from_file(const std::string &file_path);
        void set_body(const std::vector<char> &body);
        void set_version(std::string version)       {this->_version = version;};
        void set_connetion(std::string connetion)   {this->_connetion = connetion;};
        void set_content_type(std::string type)     {this->_content_type.insert(this->_content_type.end(), type);};
        void set_server(std::string name)           {this->_server = name;};
};

std::ostream &operator<<(std::ostream &os, HttpResponse &response);
