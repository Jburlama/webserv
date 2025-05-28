#include "../includes/HttpRequest.hpp"

std::string g_methods[] = {
    "GET",
    "POST",
    "DELETE",
};

HttpRequest::HttpRequest()
{
    throw std::logic_error("Provid Request string");
}

HttpRequest::HttpRequest(char *str)
{
    int state = START;

    this->_str = str;

    for (int i = 0; str[i]; ++i)
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
                this->_body = this->_parse_body(--i, str);
                state = END;
            case END:
                return ;
            default:
                break;
        }
    }
}

std::string HttpRequest::_parse_body(int &i, char *str)
{
    return str + i;
}

std::map<std::string, std::vector<std::string> > HttpRequest::_parse_header(int &i, char *str)
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

std::string HttpRequest::_parse_version(int &i, char *str)
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

std::string HttpRequest::_parse_path(int &i, char *str)
{
    std::string path;
    if (std::isspace(str[i]))
        throw std::logic_error("Error at status line: Can only have one SP between Method and URI");

    while (!std::isspace(str[i]))
        path += str[i++];
    return path;
}

std::string HttpRequest::_parse_method(int &i, char *str)
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

std::string HttpRequest::get_method()
{
    return this->_method;
}

std::string HttpRequest::get_path()
{
    return this->_path;
}

std::string HttpRequest::get_version()
{
    return this->_version;
}

std::map<std::string, std::vector<std::string> > HttpRequest::get_headers()
{
    return this->_headers;
}

std::string HttpRequest::get_body()
{
    return this->_body;
}

std::ostream &operator<<(std::ostream &os, HttpRequest &request)
{
    os << "Method: " << request.get_method() << "\n";
    os << "URI Path: " << request.get_path() << "\n";
    os << "HTTP version: " << request.get_version() << "\n";
    os << "Body: " << request.get_body() << "\n";
    return os;
}
