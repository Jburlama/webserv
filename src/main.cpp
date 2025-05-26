#include "../includes/webserv.hpp"

int main (int argc, char *argv[])
{
    (void)argc;
    (void)argv;

	try
	{
        Core server(8000);

        server.client_multiplex();
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
