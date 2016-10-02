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

char hexToByte(char x, char y) {
  return (asciiToByte(x) << 4) | asciiToByte(y);
}

std::pair<char,char> byteToHex(unsigned char x) {
    unsigned char front = x >> 4;
    unsigned char back  = x & 0xf;
    front += front >= 0 && front <= 9 ? '0' : 'A' - 10;
    back += back >= 0 && back <= 9 ? '0' : 'A' - 10;
    return std::make_pair<char,char>(front,back);
}
