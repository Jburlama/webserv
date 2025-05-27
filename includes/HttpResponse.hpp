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

class HttpResponse
{
    private:
        std::string                                         _str;
        std::string                                         _version;
        int                                                 _status_code;
        std::string                                         _description;
        std::string                                         _date;
        std::vector<std::string>                            _content_type;
        size_t                                              _content_length;
    std::string                                             _server;
        std::string                                         _body;
    public:
        HttpResponse(std::string method, int clientfd);

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
