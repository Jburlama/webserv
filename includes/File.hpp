#pragma once
#include <iterator>
#include <sys/stat.h>
#include <stdexcept>
#include <fcntl.h>
#include <unistd.h>

class File
{
    protected:
        int             _file_fd;
        struct stat     _file_stats;
        size_t          _file_bytes;
        std::string     _file_path;
        char            *_file_buffer;
    public:
        File();
        ~File();

        int         get_file_fd()       {return this->_file_fd;};
        struct stat &get_file_stats()   {return this->_file_stats;};
        size_t      &get_file_bytes()   {return this->_file_bytes;};
        std::string &get_file_path()    {return this->_file_path;};
        char        *get_file_buffer()  {return this->_file_buffer;};

        void        set_file_bytes(size_t bytes) {this->_file_bytes = bytes;};
};
