#include "abstract.hpp"


int connect_to(const char* hostname, const char* port) {
    struct addrinfo hint;
    struct addrinfo *host_info_list;
    struct addrinfo *p;
    int status;
    int socket_fd;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_UNSPEC;
    hint.ai_socktype = SOCK_STREAM;
    // get addrinfo
    status = getaddrinfo(hostname, port, &hint, &host_info_list);
    if (status != 0) {
        fprintf(stderr, "Error: cannot get address info for host when connecting\n");
        exit(EXIT_FAILURE);
    }
    // get socket descriptor
    for (p = host_info_list; p != NULL; p = p->ai_next) {
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socket_fd < 0) {
            continue;
        }
        // connect to server
        status = connect(socket_fd, p->ai_addr, p->ai_addrlen);
        // check if connect success
        if (status == -1) {
            close(socket_fd);
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "Error: fail to connect\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(host_info_list); // all done with this
    
    return socket_fd;
}


int create_service(const char * hostname, const char * port) {
    int socket_fd;
    int status;
    struct addrinfo hints;
    struct addrinfo* host_info_list;
    struct addrinfo* p;
    int yes = 1;
    memset(&hints, 0, sizeof(hints)); // initialize hint
    // get host info list
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    status = getaddrinfo(hostname, port, &hints, &host_info_list);
    if (status != 0) {
        fprintf(stderr, "Error: cannot get addrss info for host when creating\n");
        exit(EXIT_FAILURE);
    }

    // get socket descriptor
    for (p = host_info_list; p != NULL; p = p->ai_next) {
        // if there are no specific port number, randomly assign one.
        socket_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        //socket_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (socket_fd < 0) {
            continue;
        }
        // avoid message: address already in use
        if (setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            fprintf(stderr, "setsockopt\n");
            exit(EXIT_FAILURE);
        }
        status = bind(socket_fd, p->ai_addr, p->ai_addrlen);
        // check if bind success
        if (status == -1) {
            close(socket_fd);
            continue;
        }
        break;
    }
    // check if fail to check
    if (p == NULL) {
        fprintf(stderr, "Error: fail to create socket or bind\n");
        exit(EXIT_FAILURE);
    }
    // listen
    status = listen(socket_fd, 100);
    if (status == -1) {
        fprintf(stderr, "Error: fail to listen\n");
        exit(EXIT_FAILURE);
    }

    freeaddrinfo(host_info_list); // all done with this
    return socket_fd;
}


int accept_connect(int socket_fd) {
    int client_connection_fd;
    struct sockaddr_storage player_socket_addr;
    socklen_t player_socket_addr_len = sizeof(player_socket_addr);
    // accept players' connection
    client_connection_fd = accept(socket_fd, (struct sockaddr *)&player_socket_addr, &player_socket_addr_len);
    if (client_connection_fd == -1) {
        fprintf(stderr, "Error: cannot accept connection on socket\n");
        exit(EXIT_FAILURE);
    }
    return client_connection_fd;
}

void recv_all(vector<char>& all_msg, int sockt_fd) {
  char msg[65535];
  int status;
  do {
    cout << "Still Receving ...\n";
    int status = recv(sockt_fd, msg, 65535, 0);
    for (int i = 0; i < 65535; i++) all_msg.push_back(msg[i]);
  } while (status > 0);
}


void mode_connect(int client_fd, int server_fd) {
    send(client_fd, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
    fd_set main_set;
    fd_set read_fds;
    FD_ZERO(&main_set); 
    FD_ZERO(&read_fds);
    FD_SET(client_fd, &main_set);
    FD_SET(server_fd, &main_set);
    int fdmax = client_fd > server_fd ? client_fd : server_fd;
    while (1) {
        read_fds = main_set;
        if (select(fdmax + 1, &read_fds, NULL, NULL, NULL) == -1) {
            fprintf(stderr, "Error: select\n");
            exit(EXIT_FAILURE);
        }
        if (FD_ISSET(client_fd, &read_fds)) {
            cout << "client msg\n";
            char msg[65535];
            int len = recv(client_fd, &msg, 65535, 0);
            if (len <= 0) exit(EXIT_SUCCESS);
            send(server_fd, &msg, len, 0);
            //cout << msg;
        }
        else if (FD_ISSET(server_fd, &read_fds)){
            cout << "server msg\n";
            char msg[65535];
            int len = recv(server_fd, &msg, 65535, 0);
            if (len <= 0) exit(EXIT_SUCCESS);
            send(client_fd, &msg, len, 0);
            //cout << msg;
        }
    }

}

void mode_request(int client_fd, int server_fd, char * msg, int len, Request & req) {
  /* send the request from the client to the target server */
  send(server_fd, msg, len, 0);
  /* get the Response from the target server */
  vector<char> res_msg;
  recv_all(res_msg, server_fd);
  cout << "\nResponse Receving Succefully\n";
  printf("%s", res_msg.data());
  /* parsing the response */
  Parser res_par;
  res_par.parsing(res_msg);
  /* generate the http response pocket */
  Response res(res_par.getstartline(), res_par.getheaders(), res_msg);
  /* check if the Response can be cashable or not */
  // TODO

  /* send the Response back to the client */
  send(client_fd, res_msg.data(), res_msg.size(), 0);
  cout << "\nSend back to client Succefully\n";
}
