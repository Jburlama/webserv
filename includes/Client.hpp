#pragma once
#include "HttpResponse.hpp"
#include <ctime>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <stdexcept>
#include <unistd.h>

#include "HttpResponse.hpp"

class Client
{
    private:
        int                 _fd;
        time_t              _last_activity;  // Track last I/O activity
        std::vector<char>   _read_buffer;    // Incoming data
        std::vector<char>   _write_buffer;   // Outgoing data
        bool                _response_ready;
        HttpResponse        _response;
        bool                _header_sent;
        ssize_t             _bytes_sent;   // Send progress

    public:
        Client():_fd(-1),_last_activity(0),_header_sent(false) {};
        Client(int fd):_fd(fd),_response_ready(false),_header_sent(false),_bytes_sent(0) {};
        ~Client() {};

        int                 get_fd()              const {return this->_fd;};
        time_t              get_last_activity()         {return this->_last_activity;};
        std::vector<char>   &get_read_buffer()          {return this->_read_buffer;};
        std::vector<char>   &get_write_buffer()         {return this->_write_buffer;};
        HttpResponse        &get_response()             {return this->_response;};
        bool                is_response_ready()         {return this->_response_ready;};
        bool                get_header_sent()           {return this->_header_sent;};
        size_t              get_bytes_sent()            {return this->_bytes_sent;};

        void    set_last_activity()                     {this->_last_activity = time(NULL);};
        void    set_response_ready(bool ready)          {this->_response_ready = ready;};
        void    set_response(HttpResponse &response)    {this->_response = response;};
        void    set_header_sent(bool is_sent)           {this->_header_sent = is_sent;};
        void    set_bytes_sent(size_t bytes)            {this->_bytes_sent = bytes;};
};
