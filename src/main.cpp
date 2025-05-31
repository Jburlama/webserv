#include "../includes/webserv.hpp"

int main (int argc, char *argv[])
{
    (void)argc;
    (void)argv;
    std::vector<int> ports;

    ports.insert(ports.end(), 8000);
    ports.insert(ports.end(), 8001);
	try
	{
        Core core(ports);

        core.client_multiplex();
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
