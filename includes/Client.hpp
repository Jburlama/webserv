#pragma once
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <ctime>
#include <cstring>
#include <sys/types.h>
#include <vector>
#include <sys/socket.h>
#include <stdexcept>
#include <unistd.h>

#include "HttpResponse.hpp"
#include "File.hpp"

enum e_ClientState {
    BUILD_REQUEST = 1,
    BUILD_RESPONSE = 2,
    WRITING_HEADER = 3,
    WRITING_BODY = 4,
};

class Client
:public HttpRequest, public HttpResponse,
public File
{
    private:
        int                     _fd;
        time_t                  _last_activity;  // Track last I/O activity
        HttpResponse            _response;
        int                     _client_state;
        ssize_t                 _bytes_sent;   // Send progress
        File                    _file; // File to build the response body

    public:
        Client():_fd(-1),_last_activity(0),_client_state(BUILD_REQUEST),_bytes_sent(0) {};
        Client(int fd);
        ~Client() {};

        int                 get_fd()              const {return this->_fd;};
        time_t              get_last_activity()         {return this->_last_activity;};
        HttpResponse        &get_response()             {return this->_response;};
        int                 get_client_state()          {return this->_client_state;};
        size_t              get_bytes_sent()            {return this->_bytes_sent;};
        File                &get_files()                {return this->_file;};

        void    set_resquest(const char *buffer, ssize_t bytes);
        void    set_response();
        void    set_last_activity()                     {this->_last_activity = time(NULL);};
        void    set_client_state(e_ClientState state)   {this->_client_state = state;};
        void    set_response(HttpResponse response)     {this->_response = response;};
        void    set_bytes_sent(size_t bytes)            {this->_bytes_sent = bytes;};
        void    set_file(const char *path_name);
        void    set_response_body();
};
