#ifndef HTTPPROXY_H_H  
#define HTTPPROXY_H_H 

#include "abstract.hpp"
#include "Parser.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "myTime.hpp"
#include <arpa/inet.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <vector>
#include <iostream>
#include <fstream>
#include <pthread.h>
#include <ctime>
using namespace std;

struct client_t {
  int client_fd;
  int client_id;
};
typedef struct client_t client;

void build_proxy();

void handler(int sig);

void create_daemon();

void mode_connect(int client_fd, int client_id, int server_fd);

void mode_get(int client_fd, int client_id, int server_fd, char * msg, int len, Request & req, string startLine);

void mode_post(int client_fd, int client_id, int server_fd, char * msg, int len, Request & req);

void * communicate(void * client_msg);

void try_cache(Response & res, string startLine, int client_id);

bool try_used_cache(int client_fd, int client_id, int server_fd, char * msg, int len, Request & req, string startLine);

#endif  