#include "gamepad.hpp"
#include <fcntl.h>
#include <unistd.h>

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
}

void GamePad::Update() {
  struct js_event e;
  read(fd, &e, sizeof(struct js_event));

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

void GamePad::SetCarState(CarState& state) {
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
  }
  float basePwm = 100;
  float leftMod = 1;
  float rightMod = 1;
  if (LStickX > 1000) {
    // <1000 -> Go right -> slow down right wheel
    leftMod -= float(LStickX)/32768;
    if (leftMod < 0) {
      leftMod = 0;
    }
  } else if (LStickX < -1000) {
    // >1000 -> Go left -> slow down left wheel
    rightMod -= float(LStickX)/-32768;
    if (rightMod < 0) {
      rightMod = 0;
    }
  }
  if (RTrigger > -22767) {
    int modifier = float(RTrigger + 32768)/ 65536;
    basePwm = float(basePwm) * modifier;
  } else if (LTrigger > -22767) {
    int modifier = float(LTrigger + 32768)/ 65536;
    basePwm = float(basePwm) * modifier;
  } else {
    basePwm = 0;
  }
  state.RPWM = rightMod * basePwm;
  state.LPWM = leftMod * basePwm;
}
