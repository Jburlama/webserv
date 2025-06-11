#pragma once
#include <cstring>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <unistd.h>
#include <iostream>
#include <stdexcept>
#include <map>
#include <csignal>

#include "Core.hpp"
#include "HttpRequest.hpp"
#include "HttpResponse.hpp"
#include "Client.hpp"
#include "File.hpp"
#include "Log.hpp"
#include "config.hpp"

void handle_request(int client_socket);
