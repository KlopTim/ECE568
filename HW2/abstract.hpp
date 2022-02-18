#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include "Request.hpp"
#include "Response.hpp"
#include "Parser.hpp"
using namespace std;

int connect_to(const char* hostname, const char* port);

int create_service_autoPort(char *host, size_t hostlen, char *serv, size_t servlen);

int create_service(const char * hostname, const char * port);

int accept_connect(int socket_fd);

