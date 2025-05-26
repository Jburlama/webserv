#pragma once
#include <map>
#include <string>
#include <vector>

class HttpResponse
{
    private:
        std::string                                         _str;
        std::string                                         _version;
        int                                                 _status_code;
        std::string                                         _description;
        std::map<std::string, std::vector<std::string> >    _headers;
        std::string                                         _body;
    public:
        std::string                                         get_str();
        std::string                                         get_version();
        int                                                 get_status_code();
        std::string                                         get_descrition();
        std::map<std::string, std::vector<std::string> >    get_headers();
        std::string                                         get_body();

        void set_str(std::string str);
        void set_version(std::string version);
        void set_status_code(int code);
        void set_descrition(std::string description);
        void set_headers(std::map<std::string, std::vector<std::string> > headers);
        void set_body(std::string body);
};
