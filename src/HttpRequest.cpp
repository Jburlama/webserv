#include "../includes/HttpRequest.hpp"

std::string g_methods[] = {
    "GET",
    "POST",
    "DELETE",
};


HttpRequest &HttpRequest::operator=(HttpRequest &other)
{
    this->_method = other.get_method();
    this->_path = other.get_path();
    this->_request_version = other.get_request_version();
    this->_request_headers = other.get_request_headers();
    this->_request_body = other.get_request_body();
    return *this;
}


int HttpRequest::_parse_request_body() // return 201 Created or 302 Found
{
    std::string content_type;
    std::string str(&this->_request_body[0]);

    content_type = this->_request_headers["Content-Type"][0];

    if (content_type.compare("multipart/form-data") == 0)
    {
        std::string boundary;
        std::string start_boundary;
        std::string end_boundary;
        std::string filename;
        size_t      boundary_poss;
        size_t      filename_poss;
        size_t      filename_end;
        size_t      header_end;
        size_t      file_start;
        size_t      file_end;

        boundary = this->_request_headers["Content-Type"][1].c_str() + 9; // ----geckoformboundary
        boundary_poss = str.find(boundary);
        if (boundary_poss == std::string::npos)
            throw std::logic_error("Multipart boundary not found");

        start_boundary = "--" + boundary + "\r\n";
        end_boundary = "\r\n--" + boundary + "--";

        filename_poss = str.find("filename=\"");
        if (filename_poss == std::string::npos)
            throw std::logic_error("Filename not found in multipart header");
        filename_poss += 10;  // Move past 'filename="'
        filename_end = str.find('"', filename_poss);
        filename = str.substr(filename_poss, filename_end - filename_poss);
        if (filename.empty())
            return 302;
        filename = "upload/" + filename;
        this->_location = filename;

        header_end = str.find("\r\n\r\n", filename_end);
        if (header_end == std::string::npos)
            throw std::logic_error("Multipart header end not found");
        
        file_start = header_end + 4;  // Skip \r\n\r\n
        file_end = std::atoi(this->_request_headers["Content-Length"][0].c_str()); // Find end of file data (before closing boundary)
        if (file_end - file_start == 0)
            return 302;

        // Write to file
         std::ofstream out_file(filename.c_str(), std::ios::binary);
         if (!out_file)
             throw std::runtime_error("Failed to create file: " + filename);
        
         out_file.write(&this->_request_body[file_start], file_end);
         out_file.close();
    }
    return 201;
}

std::map<std::string, std::vector<std::string> > HttpRequest::_parse_request_header(int &i, const char *str)
{
    std::map<std::string, std::vector<std::string> > headers;
    std::string name;
    std::string value;
    int         lines;

    // The Header must start with a name
    if (!std::isalpha(str[i]))
        throw std::logic_error("Error header name must start with a charactor");
    while (!std::isspace(str[i]) && str[i] != ':' && str[i])
        name += str[i++];
    if (str[i++] != ':')
        throw std::logic_error("Error header name must end with ':'");
    while (str[i])
    {
        lines = 0;
        value = "";
        while (std::isspace(str[i]) && str[i])
        {
            if (str[i] == '\n')
                ++lines;
            ++i;
            if (lines == 2) // End of the Header
                return headers;
        }
        if (!str[i])
            throw std::logic_error("Error header no value after name");
        if (str[i - 1] == '\n') // if there is a line break before, we are at the start of a new header name
        {
            name = "";
            while (!std::isspace(str[i]) && str[i] != ':' && str[i])
                name += str[i++];
            if (str[i++] != ':')
                throw std::logic_error("Error header name must end with ':'");
            continue; // We go back to the beginning to clear the white spaces after the colon
        }
        while (!std::isspace(str[i]) && str[i] != ',' && str[i] != ';' && str[i])
            value += str[i++];
        headers[name].insert(headers[name].end(), value);
        ++i;
    }

    return headers;
}

std::string HttpRequest::_parse_request_version(int &i, const char *str)
{
    std::string version_start = "HTTP/";
    std::string version;
    if (std::isspace(str[i]))
        throw std::logic_error("VERSION");

    for (int j = 0; version_start[j]; ++j)
    {
        if (version_start[j] != str[i])
            throw std::logic_error("VERSION");
        version += str[i++];
    }
    if ((str[i] - '0') < 2)
        version += str[i++];
    else
        throw std::logic_error("VERSION");
    if (str[i] == '.')
        version += str[i++];
    else
        throw std::logic_error("VERSION");
    if ((str[i] - '0') < 2)
        version += str[i++];
    else
        throw std::logic_error("VERSION");
    if (!str[i])
        throw std::logic_error("VERSION");
    else if (str[i] == CR && str[i + 1] == LF)
    {
        i += 2;
        return version;
    }
    else if (str[i] == LF)
    {
        ++i;
        return version;
    }
    else
    {
        return NULL;
        throw std::logic_error("VERSION");
    }
}

std::string HttpRequest::_parse_path(int &i, const char *str)
{
    std::string path;
    if (std::isspace(str[i]))
        throw std::logic_error("PATH");

    _path_info.clear();

    // Parse path until '?' or space
    while (!std::isspace(str[i]) && str[i] != '?' && str[i])
        path += str[i++];

    // Parse query string if present
    if (str[i] == '?')
    {
        ++i;  // Skip '?'
        while (!std::isspace(str[i]) && str[i])
            _path_info += str[i++];
    }
    return path;
}

std::string HttpRequest::_parse_method(int &i, const char *str)
{
    int j = 0;

    while (str[i] == g_methods[0][j++])
        ++i;
    if (!g_methods[0][j] && str[i])
        return "GET";
    i -= --j;
    j = 0;
    while (str[i] == g_methods[1][j++])
        ++i;
    if (!g_methods[1][j] && str[i])
        return "POST";
    i -= --j;
    j = 0;
    while (str[i] == g_methods[2][j++])
        ++i;
    if (!g_methods[2][j] && str[i])
        return "DELETE";

    throw std::logic_error("Error at status line: incorrect Method");
    return NULL;
}

std::ostream &operator<<(std::ostream &os, HttpRequest &request)
{
    os << "Method: " << request.get_method() << "\n";
    os << "URI Path: " << request.get_path() << "\n";
    os << "HTTP version: " << request.get_request_version() << "\n";
    return os;
}
