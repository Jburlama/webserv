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
#include "Log.hpp"

enum e_ClientState {
    BUILD_REQUEST  = 1,
    READ_FILE      = 2,
    BUILD_RESPONSE = 3,
    WRITING_HEADER = 4,
    WRITING_BODY   = 5,
};

class Client
:public HttpRequest, public HttpResponse,
public File
{
    private:
        int                     _fd;
        time_t                  _last_activity;  // Track last I/O activity
        int                     _client_state;
        ssize_t                 _bytes_sent;   // Send progress
        int                     _status;

    public:
        Client():_fd(-1),_last_activity(0),_client_state(BUILD_REQUEST),_bytes_sent(0) {};
        Client(int fd);
        ~Client();

        int                 get_fd()              const {return this->_fd;};
        time_t              get_last_activity()         {return this->_last_activity;};
        int                 get_client_state()          {return this->_client_state;};
        ssize_t             get_bytes_sent()            {return this->_bytes_sent;};
        int                 get_status()                {return this->_status;};

        void    set_status(int status) {this->_status = status;};
        void    set_resquest(const char *buffer, ssize_t bytes);
        void    set_response();
        void    set_last_activity()                  {this->_last_activity = time(NULL);};
        void    set_client_state(int state)          {this->_client_state = state;};
        void    set_file(const char *path_name);
        void    set_response_body();
        void    set_bytes_sent(ssize_t bytes)        {this->_bytes_sent = bytes;};
        void    set_closed_fd()      {this->_fd = -1;};
        void    set_closed_file_fd() {this->_file_fd = -1;};
};
