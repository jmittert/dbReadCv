#include "server.hpp"
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <cstring>
#include <string>
#include <sstream>


Server::Server(int p) : port(p){}

int Server::Listen() {
    if (connected) {
        return 0;
    }
    memset(&hints, 0, sizeof hints);
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;

    std::stringstream ss;
    ss << port;
    const char* port_str = ss.str().c_str();
    int status;
    if ((status = getaddrinfo(NULL, port_str, &hints, &servinfo)) != 0) {
        std::cerr << "gettaddrinfo error: " << status << std::endl;
        return errno;
    }

    s = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (s == -1) {
        std::cerr << "socket error: " << errno << std::endl;
        close(s);
        return errno;
    }

    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (bind(s, servinfo->ai_addr, servinfo->ai_addrlen) == -1) {
        std::cerr << "bind error: " << errno << std::endl;
        return errno;
    }

    if (listen(s, 5) == -1) {
        std::cerr << "listen error: " << errno << std::endl;
        return errno;
    }

    socklen_t addr_size = sizeof their_addr;
    fd = accept(s, (struct sockaddr*)&their_addr , &addr_size);
    if (fd == -1) {
        std::cerr << "accept error: " << errno << std:: endl;
        return errno;
    }

    close(s);
    connected = true;
    return 0;
}

int Server::Initialize() {
    return Listen();
}

Server::~Server() {
    freeaddrinfo(servinfo);
    shutdown(fd, 2);
    close(fd);
}
