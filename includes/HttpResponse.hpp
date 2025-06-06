#pragma once
#include <cstddef>
#include <cstring>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sstream>
#include <fcntl.h>
#include <fstream>
#include <unistd.h>
#include <iostream>
#include <map>
#include <vector>

#include "HttpRequest.hpp"

class HttpResponse
{
    protected:
        std::string              _response_header; // all the Header
        std::string              _version; // HTTP/1.1
        int                      _status_code; 
        std::string              _description; // status code desc
        std::string              _date;
        std::vector<std::string> _content_type; // e.g text/html
        size_t                   _content_length; // the exact byte count of the body
        std::string              _server; // software name (webserv)
        std::string              _connection;
        char                     *_response_body; // page content in binary form
    
    public:
        HttpResponse();
        HttpResponse &operator=(HttpResponse &other);

        std::string              get_response_header()  {return this->_response_header;};
        std::string              get_version()          {return this->_version;};
        int                      get_status_code()      {return this->_status_code;};
        std::string              get_descrition()       {return this->_description;};
        std::string              get_date()             {return this->_date;};
        ssize_t                  get_content_lenght()   {return this->_content_length;};
        std::vector<std::string> get_content_type()     {return this->_content_type;};
        std::string              get_server()           {return this->_server;};
        std::string              get_connection()       {return this->_connection;};
        char                     *get_response_body()   {return this->_response_body;};

        void set_response_header();
        void set_status_code(int code);
        void set_date();
        void set_version(std::string version)       {this->_version = version;};
        void set_connection(std::string connection) {this->_connection = connection;};
        void set_content_type(std::string type)     {this->_content_type.insert(this->_content_type.end(), type);};
        void set_server(std::string name)           {this->_server = name;};
        void set_content_length(size_t size)        {this->_content_length = size;};
};

std::ostream &operator<<(std::ostream &os, HttpResponse &response);
