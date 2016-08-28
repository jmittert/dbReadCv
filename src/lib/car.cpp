#include "car.hpp"
Car::Car() {
    wiringPiSetup();
    pinMode(PIN_A1, OUTPUT);
    pinMode(PIN_A2, OUTPUT);
    pinMode(PIN_B1, OUTPUT);
    pinMode(PIN_B2, OUTPUT);
    softPwmCreate(PIN_LPWM, 0, 100);
    softPwmCreate(PIN_RPWM, 0, 100);
    state.A1 = state.A2 = state.B1 = state.B2 = state.LPWM = state.RPWM = 0;
    Write();
}

void Car::Write() {
  digitalWrite(PIN_A1, state.A1);
  digitalWrite(PIN_A2, state.A2);
  digitalWrite(PIN_B1, state.B1);
  digitalWrite(PIN_B2, state.B2);
  softPwmWrite(PIN_LPWM, state.LPWM);
  softPwmWrite(PIN_RPWM, state.RPWM);
}

Car::~Car() {
  state.A1 = state.A2 = state.B1 = state.B2 = state.LPWM = state.RPWM = 0;
  Write();
}
