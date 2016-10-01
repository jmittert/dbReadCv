#pragma once
#include <sys/socket.h>
#include <netdb.h>
#include <vector>

/**
 * A class that represents any thing that reads and writes
 * over a socket
 */
#define MAXBUFFLEN 100
class Communicator
{
  protected:
    int fd; /** The file descriptor to communicate on*/
    bool connected; /** True once the server has connected to a clinet*/

    /**
     * Connect to the socket
     *
     * @return 0 on success, errno on failure
     */
    virtual int Initialize() = 0;

    /**
     * Recv data from an attached client
     *
     * PRE: Server must be connected
     *
     * @param the expected number of bytes to receive
     * @return of received 
     */
    std::vector<unsigned char> Recv(uint64_t count);

  public:
    Communicator();

    /**
     * Send data to the client
     *
     * PRE: Server must be connected
     */
    void Send(std::vector<unsigned char> bytes);

    /**
     * Recv data from an attached client
     *
     * PRE: Server must be connected
     *
     * @return of received 
     */
    std::vector<unsigned char> Recv();

    ~Communicator();
};
