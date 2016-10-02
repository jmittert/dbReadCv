#pragma once
#include <opencv2/opencv.hpp>

// Converts two hex digits to the
// equivalent byte
// e.g. 'f', ' f' -> 255
// e.g. '1', ' f' -> 31
char hexToByte(char x, char y); 

// Converts a byte to the two hex digits
// e.g. 255 -> (f,f)
// e.g. 31 -> (1,f)
std::pair<char, char> byteToHex(unsigned char x);
