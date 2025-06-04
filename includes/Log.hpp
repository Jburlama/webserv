#pragma once
#include <iostream>

#define RESET       "\033[00m"
#define RED         "\033[91m"
#define GREEN       "\033[92m"
#define YELLOW      "\033[93m"
#define LIGHTPURPLE "\033[94m"
#define CYAN        "\033[96m"
#define LIGHTGRAY   "\033[97m"
#define PURPLE      "\033[95m"

class Log
{
    private:
    public:
        static void server_start(int fd, int port);
        static void listening();
        static void server_accept_client(int server, int client);
        static void on_read(int fd);
        static void on_write(int fd);
        static void rm_from_read(int fd);
        static void rm_from_write(int fd);
        static void building_request(int fd);
        static void building_response(int fd);
        static void open_file(int fd);
        static void close_file(int fd);
        static void sending_header(int fd);
        static void sending_body(int fd);
        static void all_sent(int fd);
        static void connetion_close(int fd);
        static void timeout(int fd);
        static void sent_with_no_body(int fd);
};
