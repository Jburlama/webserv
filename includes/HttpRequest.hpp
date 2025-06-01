#pragma once
#include <iostream>
#include <cctype>
#include <map>
#include <vector>

#define CR '\r'
#define LF '\n'

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
    private:
        std::string                                         _method;
        std::string                                         _path;
        std::string                                         _version;
        std::map<std::string, std::vector<std::string> >    _headers;
        std::vector<char>                                   _body;

        std::string                                         _parse_method(int &i, const char *str);
        std::string                                         _parse_path(int &i, const char *str);
        std::string                                         _parse_version(int &i, const char *str);
        std::map<std::string, std::vector<std::string> >    _parse_header(int &i, const char *str);
        void                                                _parse_body();

    public:
        HttpRequest();
        HttpRequest(const std::vector<char> &data);

        std::string                                         get_method() {return this->_method;};
        std::string                                         get_path() {return this->_path;};
        std::string                                         get_version() {return this->_version;};
        std::map<std::string, std::vector<std::string> >    get_headers() {return this->_headers;};
        std::vector<char>                                   get_body() {return this->_body;};
};

std::ostream &operator<<(std::ostream &os, HttpRequest &request);
