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
    GamePad gp = GamePad("/dev/input/js0");
    Server serv = Server(2718);
    if (serv.Listen()) {
        return 1;
    }
    while (1) {
        gp.Update();
        struct CarState state;
        gp.SetCarState(state);
        vector<unsigned char> pack = {
            state.A1, state.A2,
            state.B1, state.B2,
            state.LPWM, state.RPWM
        };
        cout << "sending" << endl;
        serv.Send(pack);
        cout << "sent" << endl;
        // Wait for a confirmation byte
        serv.Recv();
        cout << "rep" << endl;
    } 
}
