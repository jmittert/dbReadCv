#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pqxx/pqxx>
#include <unistd.h>
#include <client.hpp>
#include "car.hpp"
int main(int argc, char **argv)
{
  using namespace std;
  bool hw = true;
  bool train = true;
  // Parse the flags
  for (int i = 0; i < argc; ++i) {
    if (!strcmp(argv[i], "--hw=false")) {
      hw = false;
    } else if (!strcmp(argv[i], "--train=false")) {
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
  Car hwCar = Car(!hw);

  Client client = Client(serverAddr, 2718);
  if (client.Connect()) {
    return 1;
  }

  while(1) {
      struct CarState state;
      vector<unsigned char> pack = client.Recv();
      if (pack.size() != 6) {
        break;
      }
      state.A1 = pack[0];
      state.A2 = pack[1];
      state.B1 = pack[2];
      state.B2 = pack[3];
      state.LPWM = pack[4];
      state.RPWM = pack[5];

      hwCar.Update(state);
      client.Send({1});
  }
}

