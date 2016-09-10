#pragma once
#include <linux/joystick.h>
#include <cstdint>
#include <string>
#include "car.hpp"
class GamePad
{
  private:
    int fd;
    bool A;
    bool B;
    bool X;
    bool Y;
    bool Back;
    bool Start;
    bool LBumper;
    bool RBumper;
    bool RStickPress;
    bool LStickPress;
    bool Guide;
    int16_t LStickX;
    int16_t LStickY;
    int16_t RStickX;
    int16_t RStickY;
    int16_t LTrigger;
    int16_t RTrigger;
    int16_t DPadX;
    int16_t DPadY;
    const uint8_t DEADZONE = 200;
  public:
    GamePad(std::string f);
    ~GamePad();
    void Update();
    // Writes the Gamepad state to the given car state
    void SetCarState(CarState& state);
  private:
    int deadzone(int x);
};
