#pragma once
#include <sys/socket.h>
#include <netdb.h>
#include <vector>
#include <string>
#include "communicator.hpp"

/**
 * A client that a program and start and then read and write to
 */
class Client: public Communicator
{
    private:
        std::string addr; /** The address to connect to */
        struct addrinfo *servinfo;
        struct addrinfo hints;
        int port; /** The port the client will run on*/
    protected:
        int Initialize();
    public:
        Client(std::string, int port);
        Client(Client&);
        Client();

        /**
         * Initializes the client on the provided port
         *
         * @return 0 on success, errno on failure
         */
        int Connect();

        ~Client();
};
