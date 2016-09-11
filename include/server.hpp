#pragma once
#include <sys/socket.h>
#include <netdb.h>
#include <vector>
#include "communicator.hpp"

/**
 * A server that a program and start and then read and write to
 */
class Server: public Communicator
{
  private:
    struct addrinfo hints;
    struct addrinfo *servinfo;
    struct sockaddr_storage their_addr;
    int s; /** The socket file descriptor */
    const int port; /** The port the server will run on*/
  protected:
    int Initialize();
  public:
    Server(int port);

    /**
     * Initializes the server on the provided port
     *
     * @return 0 on success, errno on failure
     */
    int Listen();

    ~Server();
};
