#pragma once
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
        std::map<std::string, std::vector<std::string> >    _request_headers;
        const char                                          *_request_body;

        std::string                                         _parse_method(int &i, const char *str);
        std::string                                         _parse_path(int &i, const char *str);
        std::string                                         _parse_request_version(int &i, const char *str);
        std::map<std::string, std::vector<std::string> >    _parse_request_header(int &i, const char *str);
        void                                                _parse_request_body();

    public:
        HttpRequest():_parser_state(START) {};
        HttpRequest &operator=(HttpRequest &other);

        int                                                 get_parser_state() {return this->_parser_state;};
        std::string                                         get_method() {return this->_method;};
        std::string                                         get_path() {return this->_path;};
        std::string                                         get_request_version() {return this->_request_version;};
        std::map<std::string, std::vector<std::string> >    get_request_headers() {return this->_request_headers;};
        const char                                          *get_request_body() {return this->_request_body;};

        void    set_parser_state(int state) {this->_parser_state = state;};
};

std::ostream &operator<<(std::ostream &os, HttpRequest &request);
