#include "UnitTest++/UnitTest++.h"
#include "lib.hpp"
#include <iostream>

SUITE(Lib)
{
    TEST(Identity) {
        // What ever we send, we should get the same thing back
        for (int i = 0; i < 256; i ++) {
            std::pair<char, char> bytes = byteToHex((char)i);
            char actual = hexToByte(bytes.first, bytes.second);
            CHECK_EQUAL((char)i, actual);
        }
    }
}
