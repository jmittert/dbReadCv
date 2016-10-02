#include <string>
#include <sstream>
#include <fstream>
#include "lib.hpp"
#include "cvlib.hpp"

// Reads the data into the given Mat
void strToMat(std::string pic, cv::Mat& m) {
    std::ofstream f;
    f.open("/tmp/db.jpg");

    char prev;
    bool haveTwo = false;
    for (auto it=pic.begin(); it !=pic.end(); ++it) {
      if (*it == '\\' || *it == 'x') {
        continue;
      } else {
        if (haveTwo == false) {
          prev = *it;
          haveTwo = true;
          continue;
        }
        haveTwo = false;
        f << hexToByte(prev, *it);
      }
    }
    f.close();
    m = cv::imread( "/tmp/db.jpg", 1 );
}
