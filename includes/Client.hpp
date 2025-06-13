#pragma once
#include <cstddef>
#include <cstdio>
#include <iterator>
#include <stdexcept>
#include <sys/select.h>
#include <sys/socket.h>
#include <stdexcept>
#include <ctime>
#include <cstring>
#include <sys/types.h>
#include <vector>
#include <unistd.h>

#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "File.hpp"
#include "Log.hpp"
#include "config.hpp"

enum e_ClientState {
    BUILD_REQUEST           = 1,
    READ_FILE               = 2,
    BUILD_RESPONSE          = 3,
    WRITING_HEADER          = 4,
    WRITING_BODY            = 5,
    CGI_PROCESSING          = 6,
    BUILD_RESPONSE_FROM_CGI = 7,
};

class Client
:public HttpRequest, public HttpResponse,
public File
{
    private:
        int                     _fd;
        int                     _server_fd;
        time_t                  _last_activity;  // Track last I/O activity
        int                     _client_state;
        ssize_t                 _bytes_sent;   // Send progress
        int                     _status;
        bool                    _is_cgi;
        pid_t                   _cgi_pid;
        int                     _cgi_stdin;
        int                     _cgi_stdout;
        ssize_t                 _cgi_bytes_read;
        std::vector<char>       _cgi_output;

    public:
        Client():_fd(-1),_last_activity(0),_client_state(BUILD_REQUEST),_bytes_sent(0) {};
        Client(int fd, int server_fd);
        ~Client();

        int                 get_fd()              const {return this->_fd;};
        time_t              get_last_activity()         {return this->_last_activity;};
        int                 get_client_state()          {return this->_client_state;};
        ssize_t             get_bytes_sent()            {return this->_bytes_sent;};
        int                 get_status()                {return this->_status;};
        int                 get_server_fd()             {return this->_server_fd;};
        bool                get_is_cgi()                {return this->_is_cgi;};
        pid_t               get_cgi_pid()               {return this->_cgi_pid;};
        int                 get_cgi_stdin()             {return this->_cgi_stdin;};
        int                 get_cgi_stdout()            {return this->_cgi_stdout;};
        ssize_t             get_cgi_bytes_read()        {return this->_cgi_bytes_read;};
        std::vector<char>   &get_cgi_output()           {return this->_cgi_output;};

        void    set_status(int status) {this->_status = status;};
        void    set_resquest(const char *buffer, ssize_t bytes, ServerBlock &server);
        void    set_response();
        void    set_last_activity()               {this->_last_activity = time(NULL);};
        void    set_client_state(int state)       {this->_client_state = state;};
        void    set_file(const char *path_name);
        void    set_response_body();
        void    set_bytes_sent(ssize_t bytes)     {this->_bytes_sent = bytes;};
        void    set_closed_fd()                   {this->_fd = -1;};
        void    set_closed_file_fd()              {this->_file_fd = -1;};
        void    set_server_fd(int fd)             {this->_server_fd = fd;};
        void    set_is_cgi(bool is_cgi)           {this->_is_cgi = is_cgi;};
        void    set_cgi_pid(pid_t pid)            {this->_cgi_pid = pid;};
        void    set_cgi_stdin(int fd)             {this->_cgi_stdin = fd;};
        void    set_cgi_stdout(int fd)            {this->_cgi_stdout = fd;};
        void    set_cgi_bytes_read(ssize_t bytes) {this->_cgi_bytes_read = bytes;};
};
