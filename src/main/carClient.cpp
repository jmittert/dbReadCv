#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pqxx/pqxx>
#include <unistd.h>
#include "car.hpp"
int main(int argc, char **argv)
{
  struct addrinfo *servinfo;
  int s;
  int new_fd;
  using namespace std;
  bool hw = true;
  bool train = true;
  // Parse the flags
  for (int i = 0; i < argc; ++i) {
    if (strcmp(argv[i], "--hw=false")) {
      hw = false;
    } else if (strcmp(argv[i], "--train=false")) {
      train=false;
    }
  }

  string serverAddr;
  string dbAddr;
  string dbPass;
  string dbName;
  string dbUser;
  ifstream f("/etc/carrc");
  if (!f.good()) {
    cerr << "No ~/.carrc has been created" << endl;
    exit(1);
  }

  string tmp;
  while (f >> tmp) {
    string eq;
    f >> eq;
    if (tmp == "ServerAddr") {
      f >> serverAddr;
    } else if (tmp == "DbAddr") {
      f >> dbAddr;
    } else if (tmp == "DbPass") {
      f >> dbPass;
    } else if (tmp == "DbName") {
      f >> dbName;
    } else if (tmp == "DbUser") {
      f >> dbUser;
    }
  }

  // Try to get a connection
  int status;
  struct addrinfo hints;
  memset(&hints, 0, sizeof hints);

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  cout << "Connecting to " << serverAddr << endl;
  if ((status = getaddrinfo(serverAddr.c_str(), "2718", &hints, &servinfo)) != 0) {
    cerr << "gettaddrinfo error: " << status << endl;
    exit(1);
  }

  s = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
  if (s == -1) {
    cerr << "socket error: " << errno << endl;
    exit(1);
  }

  cout << "Socket: " << s << endl;
  if (connect(s, servinfo->ai_addr, servinfo->ai_addrlen)){
    cerr << "connect error: " << errno << endl;
    exit(1);
  }

  Car hwCar = Car();
  while(1) {
      struct CarState state;
      char pack[6];
      recv(s, pack, 6, 0);
      state.A1 = pack[0];
      state.A2 = pack[1];
      state.B1 = pack[2];
      state.B2 = pack[3];
      state.LPWM = pack[4];
      state.RPWM = pack[5];
      char conf[1] = {1};
      hwCar.Update(state);
      send(s, conf, 1, 0);
  }

  // Close the listening port
  close(s);
}

