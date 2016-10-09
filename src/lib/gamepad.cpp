#include "gamepad.hpp"
#include <fcntl.h>
#include <unistd.h>
#include <thread>

GamePad::GamePad(std::string f) {
  A = B = X = Y = false;
  Back = Start = false;
  LBumper = RBumper = false;
  RStickPress = LStickPress = Guide = false;
  LStickX = LStickY = RStickX = RStickY = 0;
  LTrigger = RTrigger = -32768;
  DPadX = DPadY = 0;
  fd = open(f.c_str(), O_RDONLY);
}

GamePad::~GamePad() {
  close(fd);
  updating = false;
}

void GamePad::StartUpdating() {
  updating = true;
  std::thread t([this](){while(updating) {this->Update();}});
  t.detach();
}

void GamePad::Update() {
    struct js_event e;
    read(fd, &e, sizeof(struct js_event));

    std::lock_guard<std::mutex> guard(lock);
    if (e.type == 1) {
      switch (e.number) {
        case 0:
          A = e.value ; break;
        case 1:
          B = e.value ; break;
        case 2:
          X = e.value ; break;
        case 3:
          Y = e.value; break;
        case 4:
          LBumper = e.value; break;
        case 5:
          RBumper = e.value; break;
        case 6:
          Back = e.value; break;
        case 7:
          Start = e.value; break;
        case 8:
          Guide = e.value; break;
        case 9:
          LStickPress = e.value; break;
        case 10:
          RStickPress = e.value; break; 
      }
    } else {
      switch (e.number) {
        case 0:
          LStickX = deadzone(e.value); break;
        case 1:
          LStickY = deadzone(e.value); break;
        case 2:
          LTrigger = deadzone(e.value); break;
        case 3:
          RStickY = deadzone(e.value); break;
        case 4:
          RStickX = deadzone(e.value); break;
        case 5:
          RTrigger = deadzone(e.value); break;
        case 6:
          DPadX = deadzone(e.value); break;
        case 7:
          DPadY = deadzone(e.value); break;
      }
    }
}

int GamePad::deadzone(int x) {
  return (x > DEADZONE || x < -1*DEADZONE) ? x : 0;
}

int calcPwm(int basePwm, int stick) {
  if (stick <= 1000) {
    return basePwm;
  }
  stick = stick*2/3;
  /*
   * We use a polynomial rather than a linear function
   * to soften the effect of lower stick values
   * let c = (2^15) - 1000
   * f(x) = ((-basePwm)/c^2)x^2 + basePwm
   */
  double c = 32768 - 1000;
  int pwm = (-1*basePwm)/(c*c) * (long(stick)*long(stick)) + basePwm;
  std::cout << pwm << std::endl;
  return pwm < 0 ? 0 : pwm;
}

void GamePad::SetCarState(CarState& state) {
  std::lock_guard<std::mutex> guard(lock);
  if (RTrigger > -22767) {
    state.A1 = HIGH;
    state.A2 = LOW;
    state.B1 = HIGH;
    state.B2 = LOW;
  } else if (LTrigger > -22767) {
    state.A1 = LOW;
    state.A2 = HIGH;
    state.B1 = LOW;
    state.B2 = HIGH;
  } else {
    state.A1 = LOW;
    state.A2 = LOW;
    state.B1 = LOW;
    state.B2 = LOW;
  }
  float basePwm = 100;
  int leftMod = 0;
  int rightMod = 0;
  if (LStickX > 1000) {
    leftMod = LStickX;
  } else if (LStickX < -1000) {
    rightMod = -1* LStickX;
  }
  if (RTrigger > -22767) {
    float modifier = float(RTrigger + 32768)/ 65536;
    basePwm = basePwm * modifier;
  } else if (LTrigger > -22767) {
    float modifier = float(LTrigger + 32768)/ 65536;
    basePwm = basePwm * modifier;
  } else {
    basePwm = 0;
  }
  state.RPWM = calcPwm(basePwm, rightMod);
  state.LPWM = calcPwm(basePwm, leftMod);
}
