#include <iostream>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>
#include <netdb.h>
#include <unistd.h>
#include <cstdint>
#include "lib.hpp"
#include "gamepad.hpp"

struct addrinfo *servinfo;
int s;
int new_fd;

void cleanup(int signal)
{
  freeaddrinfo(servinfo);
  close(s);
  shutdown(new_fd, 2);
  close(new_fd);
  exit(0);
}

int main(int argc, char **argv)
{
  using namespace std;
  signal(SIGINT, cleanup);
  // Load in the GamePad
  GamePad gp = GamePad("/dev/js0");
  while (1) {
    // Try to get a connection
    int status;
    struct addrinfo hints;
    struct sockaddr_storage their_addr;
    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((status = getaddrinfo(NULL, "2718", &hints, &servinfo)) != 0) {
      cerr << "gettaddrinfo error: " << status << endl;
      exit(1);
    }

    s = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);

    if (s == -1) {
      cerr << "socket error: " << errno << endl;
      exit(1);
    }

    int yes = 1;
    setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    if (bind(s, servinfo->ai_addr, servinfo->ai_addrlen) ==  -1) {
      cerr << "bind error: " << errno << endl;
      exit(1);
    }

    cout << "Listening"  << endl;
    if (listen(s, 5) == -1) {
      cerr << "listen error: " << errno << endl;
      exit(1);
    }

    cout << "Accepting"  << endl;
    socklen_t addr_size = sizeof their_addr;
    new_fd = accept(s, (struct sockaddr *)&their_addr, &addr_size);
    if (new_fd == -1) {
      cerr << "accept error: " << errno << endl;
      exit(1);
    }

    // Close the listening port
    close(s);

    int resp = 1;
    cout << "connected!" << endl;
    do {
      gp.Update();
      struct CarState state;
      gp.SetCarState(state);
      cout << state << endl;
      char pack[6];
      memset(pack, 0, 6);
      pack[0] = state.A1;
      pack[1] = state.A2;
      pack[2] = state.B1;
      pack[3] = state.B2;
      pack[4] = state.LPWM;
      pack[5] = state.RPWM;
      resp = send(new_fd, pack, 6, 0);
      cout << "Sent:" << int(pack[4]) << ", " << int(pack[5]) << endl;

      // Wait for a confirmation byte
      char byte[1];
      while(recv(new_fd, byte, 1, 0) == 0);
    } while (resp != 0);
  }
}
