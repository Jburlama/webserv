#pragma once
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include <ctime>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <stdexcept>
#include <unistd.h>

#include "HttpResponse.hpp"

enum e_ClientState {
    READING = 0,
    WRITING_HEADER = 1,
    WRITING_BODY = 2,
};

class Client
{
    private:
        int                 _fd;
        time_t              _last_activity;  // Track last I/O activity
        HttpRequest         _request;
        HttpResponse        _response;
        int                 _state;
        ssize_t             _bytes_sent;   // Send progress

    public:
        Client():_fd(-1),_last_activity(0),_state(READING),_bytes_sent(0) {};
        Client(int fd):_fd(fd),_state(READING),_bytes_sent(0) {};
        ~Client() {};

        int                 get_fd()              const {return this->_fd;};
        time_t              get_last_activity()         {return this->_last_activity;};
        HttpRequest         &get_request()              {return this->_request;};
        HttpResponse        &get_response()             {return this->_response;};
        int                 get_state()                 {return this->_state;};
        size_t              get_bytes_sent()            {return this->_bytes_sent;};

        void    set_last_activity()                     {this->_last_activity = time(NULL);};
        void    set_resquest(HttpRequest request)       {this->_request = request;};
        void    set_state(e_ClientState state)          {this->_state = state;};
        void    set_response(HttpResponse response)     {this->_response = response;};
        void    set_bytes_sent(size_t bytes)            {this->_bytes_sent = bytes;};
};
