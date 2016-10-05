#pragma once
#include <linux/joystick.h>
#include <cstdint>
#include <string>
#include <mutex>
#include <atomic>
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
    std::atomic<bool> updating;
    void Update();
    std::mutex lock;
    int deadzone(int x);
  public:
    GamePad(std::string f);
    GamePad(const GamePad& gp) = delete;
    ~GamePad();
    void StartUpdating();
    // Writes the Gamepad state to the given car state
    void SetCarState(CarState& state);
};
