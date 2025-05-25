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
        std::string                                         _str;
        std::string                                         _method;
        std::string                                         _path;
        std::string                                         _version;
        std::map<std::string, std::vector<std::string> >    _headers;
        std::string                                         _body;

        std::string                                         _parse_method(int &i, char *str);
        std::string                                         _parse_path(int &i, char *str);
        std::string                                         _parse_version(int &i, char *str);
        std::map<std::string, std::vector<std::string> >    _parse_header(int &i, char *str);
        std::string                                         _parse_body(int &i, char *str);

    public:
        HttpRequest();
        HttpRequest(char *str);
        std::string                                         get_method();
        std::string                                         get_path();
        std::string                                         get_version();
        std::map<std::string, std::vector<std::string> >    get_headers();
        std::string                                         get_body();
};

std::ostream &operator<<(std::ostream &os, HttpRequest &request);
