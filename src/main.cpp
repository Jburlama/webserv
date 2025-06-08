#include "../includes/webserv.hpp"

void printConfig(const configValues& config) {

        std::cout << "=== Server Block #" << "0" << " ===" << std::endl;
        std::cout << "listen: " << config.get_listen(0) << std::endl;
        std::cout << "host: " << config.get_host(0) << std::endl;
        std::cout << "serverName: " << config.get_serverName(0) << std::endl;
        std::cout << "errorPage: " << config.get_errorPage(0) << std::endl;
        std::cout << "clientMaxBodySize: " << config.get_clientMaxBodySize(0) << std::endl;
        std::cout << "root: " << config.get_root(0) << std::endl;
        std::cout << "index: " << config.get_index(0) << std::endl;

        std::cout << "  --- Location #" << "0" << " ---" << std::endl;
        std::cout << "  path: " << config.get_location_path(0, 0) << std::endl; // path is not in getter, print directly
        std::cout << "  index: " << config.get_location_index(0, 0) << std::endl;
        std::cout << "  allow_methods: " << config.get_location_allow_methods(0, 0) << std::endl;
        std::cout << "  upload_store: " << config.get_location_upload_store(0, 0) << std::endl;
        std::cout << "  cgi_pass: " << config.get_location_cgi_pass(0, 0) << std::endl;
        std::cout << "  cgi_path: " << config.get_location_cgi_path(0, 0) << std::endl;
        std::cout << "  cgi_ext: " << config.get_location_cgi_ext(0, 0) << std::endl;
        std::cout << "  root: " << config.get_location_root(0, 0) << std::endl;
        std::cout << "  autoindex: " << (config.get_location_autoindex(0, 0) ? "on" : "off") << std::endl;
}


int main (int argc, char *argv[])
{
    //(void)argc;
    //(void)argv;
	if (argc == 2){
		
		try{
		std::string configFile = argv[1];
		configValues test(configFile);
		printConfig(test);
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
