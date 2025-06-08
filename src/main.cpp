#include "../includes/webserv.hpp"

void configTesting(configValues test){
	(void)test;
}

int main (int argc, char *argv[])
{
    //(void)argc;
    //(void)argv;
	if (argc == 2){
		
		try{
		std::string configFile = argv[1];
		configValues test(configFile);
		configTesting(test);
		}
		catch(std::exception &e){
			std::cerr << "General exception within config" << std::endl;
		}
	}
	else {std::cerr << "Missing .config file" << std::endl;}

    (void)argv;
    std::vector<int> ports;

    ports.insert(ports.end(), 8000);
    ports.insert(ports.end(), 8001);
	try
	{
        Core server(ports);

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
