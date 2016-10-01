#include "client.hpp"
#include <unistd.h>
#include <iostream>
#include <sys/types.h>
#include <cstring>
#include <string>
#include <sstream>


#define MAXBUFFLEN 100
Client::Client(std::string addr, int p) : addr(addr), port(p){}

int Client::Connect() {
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
    int status = getaddrinfo(addr.c_str(), port_str, &hints, &servinfo);
    if (status != 0) {
        std::cerr << "gettaddrinfo error: " << gai_strerror(status)  << std::endl;
        return status;
    }

    fd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if (fd == -1) {
        std::cerr << "socket error: " << errno << std::endl;
        return errno;
    }

    std::cerr << "Connecting..." << std::endl;
    int i = 0;
    while ((i = connect(fd, servinfo->ai_addr, servinfo->ai_addrlen))) {
        int err = errno;
        std::cerr << "Ret: " << i << " Errno: " << err << std::endl;
        sleep(1);
    }

    connected = true;
    return 0;
}

int Client::Initialize() {
    return Connect();
}

Client::~Client() {
  freeaddrinfo(servinfo);
  shutdown(fd, 2);
  close(fd);
}
