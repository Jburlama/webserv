#include "../includes/File.hpp"

File::File()
:_file_fd(-1), _file_stats(), _file_bytes(0),
_file_path(""), _file_buffer(NULL)
{
}

File::~File()
{
    close(this->_file_fd);
}
