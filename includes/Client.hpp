#pragma once
#include <ctime>
#include <cstring>
#include <vector>
#include <sys/socket.h>
#include <stdexcept>
#include <unistd.h>

class Client
{
    private:
        int                 _fd;
        time_t              _last_activity;  // Track last I/O activity
        std::vector<char>   _read_buffer;    // Incoming data
        std::vector<char>   _write_buffer;   // Outgoing data
        size_t              _write_offset;   // Send progress
        bool                _response_ready;
    public:
        Client():_fd(-1),_last_activity(0) {};
        Client(int fd):_fd(fd),_response_ready(false) {};
        ~Client() {};

        int                 get_fd() const {return this->_fd;};
        time_t              get_last_activity() {return this->_last_activity;};
        std::vector<char>   &get_read_buffer() {return this->_read_buffer;};
        std::vector<char>   &get_write_buffer() {return this->_write_buffer;};
        size_t              &get_write_offset() {return this->_write_offset;};
        bool                is_response_ready() {return this->_response_ready;};

        void set_last_activity() {this->_last_activity = time(NULL);};
        void set_response_ready(bool ready) {this->_response_ready = ready;};
};
