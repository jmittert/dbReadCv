#pragma once
#include <cstdint>
#include <wiringPi.h>
#include <softPwm.h>
#include <iostream>

// The GPIO pins the motors are attached to
#define PIN_A1 0
#define PIN_A2 1
#define PIN_B1 3
#define PIN_B2 4
#define PIN_LPWM 5
#define PIN_RPWM 6

#define HIGH 1
#define LOW 0

struct CarState {
    uint8_t A1;
    uint8_t A2;
    uint8_t B1;
    uint8_t B2;
    uint8_t LPWM;
    uint8_t RPWM;
};

std::ostream& operator<<(std::ostream& os, CarState& state);

class Car
{
  private:
    CarState state;

    // Write writes the state to hardware
    void Write();
  public:
    Car();

    // Sets the car state based on newState and writes it to hardware
    void Update(CarState& newState);

    ~Car();
};
