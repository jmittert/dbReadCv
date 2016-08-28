#pragma once
#include <string>
#include <linux/joystick.h>
#include <fcntl.h>
#include <cstdint>
class GamePad {
  int fd;
  const int DEADZONE = 200;
  GamePad(std::string f) {
    fd = open(f, O_RDONLY);
  }
  ~Xboxctrl() {
    close(fd);
  }
};
