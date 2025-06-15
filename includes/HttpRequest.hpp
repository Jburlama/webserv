#pragma once
#include <cstddef>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>
#include <iostream>
#include <cctype>
#include <map>
#include <vector>
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <fstream>

#define CR '\r'
#define LF '\n'
#define CRLF "\r\n"

enum e_state {
    START = 0,
    METHOD = 1,
    PATH = 2,
    VERSION = 3,
    HEADER = 4,
    BODY = 5,
    END = 6,
};

class HttpRequest
{
    protected:
        int                                                 _parser_state;
        std::string                                         _method;
        std::string                                         _path;
        std::string                                         _request_version;
        std::string                                         _location;
        std::map<std::string, std::vector<std::string> >    _request_headers;
        std::vector<char>                                   _request_body;
        bool                                                _has_body;
        size_t                                              _bytes_read;

        std::string                                         _parse_method(int &i, const char *str);
        std::string                                         _parse_path(int &i, const char *str);
        std::string                                         _parse_request_version(int &i, const char *str);
        std::map<std::string, std::vector<std::string> >    _parse_request_header(int &i, const char *str);
        int                                                _parse_request_body();

    public:
        HttpRequest():_parser_state(START),_has_body(false) {};
        HttpRequest &operator=(HttpRequest &other);

        int                                                 get_parser_state() {return this->_parser_state;};
        std::string                                         get_method() {return this->_method;};
        std::string                                         get_path() {return this->_path;};
        std::string                                         get_location() {return this->_location;};
        std::string                                         get_request_version() {return this->_request_version;};
        std::map<std::string, std::vector<std::string> >    get_request_headers() {return this->_request_headers;};
        std::vector<char>                                   &get_request_body() {return this->_request_body;};
        bool                                                get_has_body() {return this->_has_body;};
        size_t                                              get_bytes_read() {return this->_bytes_read;};

        void    set_parser_state(int state) {this->_parser_state = state;};
        void    set_has_body(bool has) {this->_has_body = has;};
        void    set_bytes_read(size_t bytes) {this->_bytes_read = bytes;};
        void    set_location(std::string location) {this->_location = location;};
};

std::ostream &operator<<(std::ostream &os, HttpRequest &request);
