#include "../includes/Log.hpp"

void Log::server_start(int fd, int port)
{
    std::cout << YELLOW"Server started with fd " << fd;
    std::cout << ", listening on port " << port << "\n" RESET;
}

void Log::listening()
{
    std::cout << YELLOW"Select checking if fds is ready\n" RESET;
}

void Log::server_accept_client(int server, int client)
{
    std::cout << GREEN"Connetion accepted from client " << client;
    std::cout << " to server " << server << "\n" RESET;
}

void Log::on_read(int fd)
{
    std::cout << LIGHTPURPLE"Added fd " << fd << " to read set\n" RESET;
}

void Log::on_write(int fd)
{
    std::cout << LIGHTPURPLE"Added fd " << fd << " to write set\n" RESET;
}

void Log::rm_from_read(int fd)
{
    std::cout << LIGHTPURPLE"Removed fd " << fd << " to read set\n" RESET;
}

void Log::rm_from_write(int fd)
{
    std::cout << LIGHTPURPLE"Removed fd " << fd << " to write set\n" RESET;
}

void Log::building_request(int fd)
{
    std::cout << LIGHTGRAY"Building request from " << fd << "\n" RESET;
}

void Log::building_response(int fd)
{
    std::cout << LIGHTGRAY"Building response for " << fd << "\n" RESET;
}

void Log::open_file(int fd)
{
    std::cout << CYAN"Open file " << fd << "\n" RESET;
}

void Log::close_file(int fd)
{
    std::cout << CYAN"Close file " << fd << "\n" RESET;
}

void Log::sending_header(int fd)
{
    std::cout << PURPLE"Sending Header to " << fd << "\n" RESET;
}

void Log::sending_body(int fd)
{
    std::cout << PURPLE"Sending body to " << fd << "\n" RESET;
}

void Log::all_sent(int fd)
{
    std::cout << PURPLE"All sent to " << fd << "\n" RESET;
}

void Log::connetion_close(int fd)
{
    std::cout << RED"Connetion closed with " << fd << "\n" RESET;
}


void Log::timeout(int fd)
{
    std::cout << RED"Connetion timeout with " << fd << "\n" RESET;
}

void Log::sent_with_no_body(int fd)
{
    std::cout << PURPLE"Response sent with no body to " << fd << "\n" RESET;
}
