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

static std::pair<char,char> byteToAscii(unsigned char x) {
    unsigned char front = x >> 4;
    unsigned char back  = x & 0xf;
    front += front >= 0 && front <= 9 ? '0' : 'A' - 10;
    back += back >= 0 && back <= 9 ? '0' : 'A' - 10;
    return std::make_pair<char,char>(front,back);
}

int main(int argc, char **argv)
{
  using namespace std;
  bool hw = true;
  bool train = true;
  bool takePic = true;
  // Parse the flags
  for (int i = 0; i < argc; ++i) {
    if (!strcmp(argv[i], "--hw=false")) {
      hw = false;
    } else if (!strcmp(argv[i], "--train=false")) {
      train=false;
    }  else if (!strcmp(argv[i], "--pic=false")) {
      takePic=false;
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
  Car hwCar = Car(!hw);

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
    } else {
      // eat the remaining
      f >> tmp;
    }
  }
  pqxx::connection c(
      "dbname=" + dbName + " "
      "user=" + dbUser + " "
      "host=" + dbAddr);

  // Track the current and list 4 state ids, default to 0
  int ids[5] = {1,1,1,1,1};
  int currIdPtr = 0;

  // Connect to the controlling server
  Client client = Client(serverAddr, 2718);
  if (client.Connect()) {
    return 1;
  }

  // Connect to the controlling server
  Client picClient = Client("127.0.0.1", 3141);
  if (takePic) {
    cout << "connecting to camera service" << endl;
    if (picClient.Connect()) {
      return 1;
    }
    cout << "connected to camera service" << endl;
  }

  while(1) {
      pqxx::work txn(c);
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

      if (train) {
        stringstream insertState;
        insertState << "INSERT INTO states (a1, a2, b1, b2, lpwm, rpwm)"
          << " VALUES("
          << int(state.A1) << ", "
          << int(state.A2) << ", "
          << int(state.B1) << ", " 
          << int(state.B2) << ", "
          << int(state.LPWM) << ", "
          << int(state.RPWM) << ") "
          << " ON CONFLICT ON CONSTRAINT uniq"
          << " DO UPDATE SET id=states.id RETURNING id;";
        const int id = txn.exec(insertState.str())[0][0].as<int>(0);
        ids[currIdPtr] = id;
      }

      hwCar.Update(state);

      if (train) {
        // Get the picture
        vector<unsigned char> pic;
        if (takePic) {
          picClient.Send({1});
          pic = picClient.Recv();
        } else {
          pic = {' '};
        }
        stringstream ss;
        ss << "\\x";
        cout << "\\x";
        for (auto& c : pic) {
          std::pair<char,char> nibbles = byteToAscii(c);
          cout << nibbles.first << nibbles.second;
          ss << nibbles.first << nibbles.second;
        }
        cout << endl;

        string picStr = ss.str();

        cout << "ptr: " << currIdPtr << endl;
        std::string insertImg =
          "INSERT INTO images (image, state1, state2, state3, state4, state5)"
          " VALUES(" +
          txn.quote(picStr) + ", " + 
          txn.quote(ids[currIdPtr]) + ", " + 
          txn.quote(ids[(currIdPtr + 1) % 5]) + ", " +
          txn.quote(ids[(currIdPtr + 2) % 5]) + ", " +
          txn.quote(ids[(currIdPtr + 3) % 5]) + ", " +
          txn.quote(ids[(currIdPtr + 4) % 5]) + ");";
        cout << insertImg << endl;;
        txn.exec(insertImg);
        currIdPtr = (currIdPtr + 1) % 5;
      }
      client.Send({1});
      txn.commit();
  }
}
