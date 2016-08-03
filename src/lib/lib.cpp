#include <string>
#include <sstream>
#include <fstream>
#include "lib.hpp"

static int asciiToByte(char x) {
  if (x >= '0' && x <= '9') {
    return x - '0';
  } else if (x >= 'A' && x <= 'F') {
    return x + 10 - 'A';
  } else if (x >= 'a' && x <= 'f') {
    return x + 10 - 'a' ;
  }
  std::stringstream ss;
  ss << "Can't encode character: " << x << std::endl;
  throw ss.str();
}

static char hexToByte(char x, char y) {
  return (asciiToByte(x) << 4) | asciiToByte(y);
}

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
