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
        //fprintf(stderr, "Error: cannot get address info for host when connecting\n");
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
        //fprintf(stderr, "Error: fail to connect\n");
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
        //fprintf(stderr, "Error: cannot get addrss info for host when creating\n");
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
            //fprintf(stderr, "setsockopt\n");
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
        //fprintf(stderr, "Error: fail to create socket or bind\n");
        exit(EXIT_FAILURE);
    }
    // listen
    status = listen(socket_fd, 100);
    if (status == -1) {
        //fprintf(stderr, "Error: fail to listen\n");
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
        //fprintf(stderr, "Error: cannot accept connection on socket\n");
        exit(EXIT_FAILURE);
    }
    return client_connection_fd;
}


