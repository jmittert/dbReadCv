#include <string.h>
#include <iostream>
#include <fstream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pqxx/pqxx>
#include <unistd.h>
#include <client.hpp>
#include <ctime>
#include <thread>
#include <mutex>
#include "car.hpp"
#include "lib.hpp"

Car hwCar(true);
struct CarState state;
Client client;
std::mutex state_mutex;

void updateLoop() {
    while(1) {
          client.Send({1});
          std::vector<unsigned char> pack = client.Recv();
          std::lock_guard<std::mutex> lk(state_mutex);
          state.A1 = pack[0];
          state.A2 = pack[1];
          state.B1 = pack[2];
          state.B2 = pack[3];
          state.LPWM = pack[4];
          state.RPWM = pack[5];
          hwCar.Update(state);
      }
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
  string camAddr;
  ifstream f("/etc/carrc");
  if (!f.good()) {
    cerr << "No ~/.carrc has been created" << endl;
    exit(1);
  }
  hwCar = Car(!hw);

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
    } else if (tmp == "CamAddr") {
      f >> camAddr;
    } else {
      // eat the remaining
      f >> tmp;
    }
  }
  pqxx::connection c(
      "dbname=" + dbName + " "
      "user=" + dbUser + " "
      "host=" + dbAddr);

  // Track the current and last 4 state ids, default to 0
  int ids[5] = {1,1,1,1,1};
  int currIdPtr = 0;

  // Connect to the controlling server
  client = Client(serverAddr, 2718);
  if (client.Connect()) {
    return 1;
  }

  // Connect to the controlling server
  Client picClient(camAddr, 3141);
  if (takePic) {
    cout << "connecting to camera service" << endl;
    if (picClient.Connect()) {
      return 1;
    }
    cout << "connected to camera service" << endl;
  }

  std::thread t(updateLoop);
  t.detach();

  while(1) {
    if (train) {
      // Get the picture from the cam server
      vector<unsigned char> pic;
      if (takePic) {
        picClient.Send({1});
        pic = picClient.Recv();
      } else {
        pic = {' '};
      }
      stringstream ss;
      ss << "\\x";
      for (auto& c : pic) {
        std::pair<char,char> nibbles = byteToHex(c);
        ss << nibbles.first << nibbles.second;
      }
      string picStr = ss.str();

      // Get the current state
      std::unique_lock<std::mutex> lk(state_mutex);
      CarState local_state = state;
      lk.unlock();

      stringstream insertState;
      insertState << "INSERT INTO states (a1, a2, b1, b2, lpwm, rpwm)"
        << " VALUES("
        << int(local_state.A1) << ", "
        << int(local_state.A2) << ", "
        << int(local_state.B1) << ", " 
        << int(local_state.B2) << ", "
        << int(local_state.LPWM) << ", "
        << int(local_state.RPWM) << ") "
        << " ON CONFLICT ON CONSTRAINT uniq"
        << " DO UPDATE SET id=states.id RETURNING id;";
      pqxx::work txn(c);
      const int id = txn.exec(insertState.str())[0][0].as<int>(0);
      ids[currIdPtr] = id;


      std::string insertImg =
        "INSERT INTO images (image, state1, state2, state3, state4, state5)"
        " VALUES(" +
        txn.quote(picStr) + ", " + 
        txn.quote(ids[currIdPtr]) + ", " + 
        txn.quote(ids[(currIdPtr + 1) % 5]) + ", " +
        txn.quote(ids[(currIdPtr + 2) % 5]) + ", " +
        txn.quote(ids[(currIdPtr + 3) % 5]) + ", " +
        txn.quote(ids[(currIdPtr + 4) % 5]) + ");";
      txn.exec(insertImg);
      currIdPtr = (currIdPtr + 1) % 5;
      txn.commit();
    }
  }
}
