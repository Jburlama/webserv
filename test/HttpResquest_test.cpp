#include "../includes/webserv.hpp"

int main ()
{
	try
	{
        char buffer[1024];
        std::string msg = "\n  \r\nDELETE /path/to/file/index.html HTTP/1.0\r\n"
            "Header-a: some-long-value-1a, some-long-value-1b\r\n"
            "Header-b: some-long-value-2a, some-long-value-2b\r\n"
            "HEADER-c:    some-long-value-3a,\n"
            "            some-long-value-3bi\r\n\r\n";


        memset(buffer, 0, sizeof(buffer));
        memcpy(buffer, msg.c_str(), strlen(msg.c_str()));

        HttpRequest request(buffer);

        std::cout << "Method: " << request.get_method() << "\n";
        std::cout << "Path: " << request.get_path() << "\n";
        std::cout << "Version: " << request.get_version() << "\n";

        std::map<std::string, std::vector<std::string> > header;

        header = request.get_headers();

        std::cout << "--- Headers ---\n";
        std::cout << "Header-a: ";
        for (std::vector<std::string>::iterator it = header["Header-a"].begin(); it != header["Header-a"].end(); ++it)
            std::cout << *it << ", ";
        std::cout << "\n";;

        std::cout << "Header-b: ";
        for (std::vector<std::string>::iterator it = header["Header-b"].begin(); it != header["Header-b"].end(); ++it)
            std::cout << *it << ", ";
        std::cout << "\n";;

        std::cout << "HEADER-c: ";
        for (std::vector<std::string>::iterator it = header["HEADER-c"].begin(); it != header["HEADER-c"].end(); ++it)
            std::cout << *it << ", ";
        std::cout << "\n";;

        std::cout << "--- Body ---\n";
        std::cout << "Body: " << request.get_body() << "\n";
	}
	catch (std::runtime_error &e)
	{
        perror(e.what());
        exit(errno);
	}
	catch (std::logic_error &e)
	{
        std::cerr << e.what() << "\n";
        exit(STDERR_FILENO);
	}
}
