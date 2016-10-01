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
    uint64_t remaining = bytes.size();
    // Send eight bytes indicating the size, then the actual data
    // Start from the most significant and work down
    unsigned char size_bytes[8] = {0};
    for (int i = 7; i > 0; i--) {
        size_bytes[7-i] = (remaining & (1 << i)) >> i;
    }
    send(fd, &size_bytes, 8, 0);
    while (remaining) {
        remaining -= send(fd, &bytes[bytes.size() - remaining], remaining, 0);
    }
}

std::vector<unsigned char> Communicator::Recv() {
    if (!connected) {
        throw "Not connected to receiver";
    }
    // Receive the first 8 bytes indicating the size, then receive
    // the actual data
    auto size_bytes = Recv(8);
    uint64_t size = 0;
    for (int i = 0; i < 8; i++) {
        size |= (size_bytes[i] << (7-i));
    }
    return Recv(size);
}

std::vector<unsigned char> Communicator::Recv(uint64_t count) {
    if (!connected) {
        throw "Not connected to sender";
    }
    std::vector<unsigned char> data;
    char bytes[count];
    while (count > 0) {
        int num_bytes = recv(fd, bytes, count, 0);
        data.insert(data.end(), bytes, bytes+num_bytes);
        count -= num_bytes;
    }
    return data;
}

Communicator::~Communicator() {
    shutdown(fd, 2);
    close(fd);
}
