#include "../includes/HttpRequest.hpp"
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <iostream>
#include <fstream>

std::string g_methods[] = {
    "GET",
    "POST",
    "DELETE",
};

HttpRequest::HttpRequest()
{
    throw std::logic_error("Provid Request string");
}

HttpRequest::HttpRequest(const std::vector<char> &data)
{
    int state = START;

    const char *str = &data[0];

    for (int i = 0; i < (int)data.size(); ++i)
    {
        switch (state)
        {
            case START:  // skips empty line, and check if method start at the beguining of the line
                while (std::isspace(str[i]))
                    ++i;
                if ((i == 0 && (str[i] == 'G' || str[i] == 'P' || str[i] == 'D'))
                || (str[i - 1] == '\n' && (str[i] == 'G' || str[i] == 'P' || str[i] == 'D')))
                    state = METHOD;
                else
                    throw std::logic_error("Error at status line: Method must start at the beguinning of the line");
                break ;
            case METHOD:
                this->_method = this->_parse_method(--i, str);
                state = PATH;
                break;
            case PATH:
                this->_path = this->_parse_path(i, str);
                state = VERSION;
                break ;
            case VERSION:
                this->_version = this->_parse_version(i, str);
                state = HEADER;
                break ;
            case HEADER:
                this->_headers = this->_parse_header(--i, str);
                state = BODY;
                break ;
            case BODY:
                size_t body_start;
                
                body_start = --i;
                this->_body.insert(this->_body.end(), data.begin() + body_start, data.end());
                if (this->_body.size() != 0)
                    this->_parse_body();
                state = END;
            case END:
                return ;
            default:
                break;
        }
    }
}

    // "multipart/form-data";
//------geckoformboundary4ea4b54d72f9e8748e9fe720d148cfd
//Content-Disposition: form-data; name="file"; filename="The-Amazing-Power-of-Self-Discipline_-Miyamoto-Musashi.webp"
//Content-Type: image/webp
void HttpRequest::_parse_body()
{
    std::string content_type;
    std::string str(&this->_body[0]);

    content_type = this->_headers["Content-Type"][0];

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

        boundary = this->_headers["Content-Type"][1].c_str() + 9; // ----geckoformboundary
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
            return ;

        header_end = str.find("\r\n\r\n", filename_end);
        if (header_end == std::string::npos)
            throw std::logic_error("Multipart header end not found");
        
        file_start = header_end + 4;  // Skip \r\n\r\n
        file_end = std::atoi(this->_headers["Content-Length"][0].c_str()); // Find end of file data (before closing boundary)       
        std::cout << "File End: " << file_end << "\n";

        //// Write to file
        std::ofstream out_file(filename.c_str(), std::ios::binary);
        if (!out_file)
            throw std::runtime_error("Failed to create file: " + filename);
        
        out_file.write(&this->_body[file_start], file_end);
        out_file.close();
    }
}

std::map<std::string, std::vector<std::string> > HttpRequest::_parse_header(int &i, const char *str)
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

std::string HttpRequest::_parse_version(int &i, const char *str)
{
    std::string version_start = "HTTP/";
    std::string version;
    if (std::isspace(str[i]))
        throw std::logic_error("Error at status line: Can only have one SP between Method and URI");

    for (int j = 0; version_start[j]; ++j)
        version += str[i++];
    if ((str[i] - '0') <= 2)
        version += str[i++];
    else
        throw std::logic_error("Error at status line version");
    if (str[i] == '.')
        version += str[i++];
    else
        throw std::logic_error("Error at status line version");
    if ((str[i] - '0') <= 2)
        version += str[i++];
    else
        throw std::logic_error("Error at status line version");
    if (!str[i])
        throw std::logic_error("Error at status line: Cant end the status line with nul");
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
        throw std::logic_error("Error at the end of status line");
    }
}

std::string HttpRequest::_parse_path(int &i, const char *str)
{
    std::string path;
    if (std::isspace(str[i]))
        throw std::logic_error("Error at status line: Can only have one SP between Method and URI");

    while (!std::isspace(str[i]))
        path += str[i++];
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
    os << "HTTP version: " << request.get_version() << "\n";
    return os;
}
