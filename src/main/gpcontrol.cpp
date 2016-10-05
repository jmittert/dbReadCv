#include <iostream>
#include <string>
#include <fstream>
#include <sys/types.h>
#include <signal.h>
#include <cstdint>
#include "lib.hpp"
#include "server.hpp"
#include "gamepad.hpp"

int main(int argc, char **argv)
{
    using namespace std;
    // Load in the GamePad
    GamePad gp("/dev/input/js0");
    gp.StartUpdating();
    Server serv = Server(2718);
    if (serv.Listen()) {
        return 1;
    }
    while (1) {
        serv.Recv();
        struct CarState state;
        gp.SetCarState(state);
        vector<unsigned char> pack = {
            state.A1, state.A2,
            state.B1, state.B2,
            state.LPWM, state.RPWM
        };
        serv.Send(pack);
        // Wait for a confirmation byte
    } 
}
