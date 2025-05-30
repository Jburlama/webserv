#include "../includes/webserv.hpp"

void configTesting(configValues test){
	double lis = test.get_listen();
	std::cout << "Listen: " << lis << std::endl; //8080
	std::string str = test.get_host();
	std::cout << "host: " << str << std::endl; //127.0.0.1
	str = test.get_serverName();
	std::cout << "serverName: " << str << std::endl; //localhost
	str = test.get_errorPage();
	std::cout << "ErrorPage " << str << std::endl; //404 /404.html
	str = test.get_clientMaxBodySize();
	std::cout << "ClientMaxBodySize: " << str << std::endl; //1024
	str = test.get_root();
	std::cout << "root: " << str << std::endl; // /var/www/html
	str = test.get_index();
	std::cout << "index: " << str << std::endl;

	std::cout << "---------------" << std::endl;
	std::cout << "LOCATION PART" << std::endl;
	str = test.get_location_index();
	std::cout << "_index: " << str << std::endl;
	str = test.get_location_allow_methods();
	std::cout << "allow_methods: " << str << std::endl;
	str = test.get_location_upload_store();
	std::cout << "upload_store: " << str << std::endl;
	str = test.get_location_cgi_pass();
	std::cout << "cgi_pass: " << str << std::endl;
	str = test.get_location_cgi_path();
	std::cout << "cgi_path: " << str << std::endl;
	str = test.get_location_cgi_ext();
	std::cout << "cgi_ext: " << str << std::endl;
	str = test.get_location_root();
	std::cout << "root: " << str << std::endl;
	bool _bool = false;
	_bool = test.get_location_autoindex();
	std::cout << "autoIndex: " << _bool << std::endl;
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
