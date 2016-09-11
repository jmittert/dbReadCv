#include "communicator.hpp"
#include <unistd.h>
#include <iostream>

Communicator::Communicator(){
    connected=false;
}

void Communicator::Send(std::vector<unsigned char> bytes) {
    if (!connected) {
        return;
    }
    int remaining = bytes.size();
    while (remaining) {
        remaining -= send(fd, &bytes[bytes.size() - remaining], remaining, 0);
    }
}

std::vector<unsigned char> Communicator::Recv() {
    if (!connected) {
        return std::vector<unsigned char>();
    }
    std::vector<unsigned char> data;
    char bytes[MAXBUFFLEN];
    int num_bytes = recv(fd, bytes, MAXBUFFLEN - 1, 0);
    if (num_bytes == -1) {
        return data;
    }
    data.insert(data.end(), bytes, bytes+num_bytes);
    return data;
}

Communicator::~Communicator() {
    shutdown(fd, 2);
    close(fd);
}
