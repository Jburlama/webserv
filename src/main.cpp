#include "../includes/webserv.hpp"
#include <exception>
#include <stdexcept>

void printConfig(const configValues& config) {

        std::cout << "=== Server Block #" << "0" << " ===" << std::endl;
        std::cout << "listen: " << config.get_listen(0) << std::endl;
        std::cout << "host: " << config.get_host(0) << std::endl;
        std::cout << "serverName: " << config.get_serverName(0) << std::endl;
		
		/* if (!config.get_errorPage(0).empty())
    		return ;
		else{
			const std::vector<std::string>& errPages = config.get_errorPage(0);
			std::cout << "errorPage(s):" << std::endl;
			for (size_t i = 0; i < errPages.size(); ++i) {
			    std::cout << "  " << errPages[i] << std::endl;
			}
		} */
        /* std::cout << "clientMaxBodySize: " << config.get_clientMaxBodySize(0) << std::endl;
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
        std::cout << "  autoindex: " << (config.get_location_autoindex(0, 0) ? "on" : "off") << std::endl; */
		
}

void signalHandler(int signum){
    std::cout << "\nInterrupt signal received: " << signum << std::endl;
    // Cleanup function will be executed here
    exit(signum);
}


int main (int argc, char *argv[])
{
    (void)argc;
    std::vector<int> ports;
    std::signal(SIGINT, signalHandler);

    ports.insert(ports.end(), 8000);
    ports.insert(ports.end(), 8001);
	try
	{
        std::string configFile = argv[1];

        configValues config(configFile);
		printConfig(config);
        
        
        Core core(config.get_server_blocks());

        core.client_multiplex();
	}
	catch (std::runtime_error &e)
	{
        perror(e.what());
        exit(STDERR_FILENO);
	}
	catch (std::logic_error &e)
	{
        std::cerr << e.what() << "\n";
        exit(STDERR_FILENO);
	}
	catch (std::exception &e) //THIS WILL STOP THE LOOP "Probably try{}catch{} to create config before the Core"
	{
		exit(STDERR_FILENO);
	}
}
