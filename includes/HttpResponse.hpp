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

#include "HttpRequest.hpp"

class HttpResponse
{
    private:
        std::string                                         _str; // All the data sent
        std::string                                         _version; // HTTP/1.1
        int                                                 _status_code; 
        std::string                                         _description; // status code desc
        std::string                                         _date;
        std::vector<std::string>                            _content_type; // e.g text/html
        size_t                                              _content_length; // the exact byte count of the body
        std::string                                         _server; // software name (webserv)
        std::string                                         _body; // page content
    public:
        HttpResponse(HttpRequest &request, int clientfd);

        std::string                                         get_str();
        std::string                                         get_version();
        int                                                 get_status_code();
        std::string                                         get_descrition();
        std::string                                         get_date();
        size_t                                              get_content_lenght();
        std::vector<std::string>                            get_content_type();
        std::string                                         get_server();
        std::string                                         get_body();

        void set_str();
        void set_version(std::string version);
        void set_status_code(int code);
        void set_date();
        void set_content_type(std::string type);
        void set_server(std::string name);
        void set_body(std::string body);
};
