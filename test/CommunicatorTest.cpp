#include "UnitTest++/UnitTest++.h"
#include "communicator.hpp"
#include "server.hpp"
#include "client.hpp"
#include <unistd.h>
#include <iostream>

SUITE(Communicator)
{
    TEST(Identity) {
        // What ever we send, we should get the same thing back
        pid_t pid = fork();
        if (pid == 0) {
            Server s(9001);
            std::cout << "Parent" << std::endl;
            s.Listen();
            s.Send({1,2,3,4,5});
            std::vector<unsigned char> bytes = s.Recv();
            std::vector<unsigned char> exp = {5,4,3,2,1};
            CHECK(exp == bytes);
        } else {
            Client c("127.0.0.1", 9001); 
            std::cout << "Child" << std::endl;
            c.Connect();
            auto bytes = c.Recv();
            std::vector<unsigned char> exp = {1,2,3,4,5};
            CHECK(exp == bytes);
            c.Send({5,4,3,2,1});
        }
    }
}
