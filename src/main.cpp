#include "../includes/webserv.hpp"

void signalHandler(int signum){
    (void)signum;
    throw std::exception();
}


int main (int argc, char *argv[], char *env[])
{
    std::signal(SIGINT, signalHandler);

	try
	{
        std::string configFile;
        if (argc == 1)
            configFile = "config/default.conf";
        else
            configFile = argv[1];

        configValues config(configFile);
        
        Core core(config.get_server_blocks());

        core.client_multiplex(env);
	}
	catch (std::runtime_error &e)
	{
        perror(e.what());
        return 2;
	}
	catch (std::logic_error &e)
	{
        std::cerr << e.what() << "\n";
        return 2;
	}
	catch (std::exception &e) //THIS WILL STOP THE LOOP "Probably try{}catch{} to create config before the Core". Check how this is supposed to be handled
	{
        return 2;
	}
}
