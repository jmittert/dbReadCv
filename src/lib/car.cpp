#include "car.hpp"
#include <cstring>
#include <iostream>

std::ostream& operator<<(std::ostream& os, CarState& state) {
    os << "CarState: {"
        " A1: " << unsigned(state.A1) << 
        " A2: " << unsigned(state.A2) << 
        " B1: " << unsigned(state.B1) << 
        " B2: " << unsigned(state.B2) << 
        " LPWM: " << unsigned(state.LPWM) << 
        " RPWM: " << unsigned(state.RPWM) << 
        " }";
    return os;
}

Car::Car() {
    fake = false;
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

Car::Car(bool f): fake(f) {
    if (fake) {
        state.A1 = state.A2 = state.B1 = state.B2 = state.LPWM = state.RPWM = 0;
        Write();
    } else {
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
}

void Car::Write() {
    if (fake) {
      std::cout << "Update: " << state << std::endl;
    } else {
      digitalWrite(PIN_A1, state.A1);
      digitalWrite(PIN_A2, state.A2);
      digitalWrite(PIN_B1, state.B1);
      digitalWrite(PIN_B2, state.B2);
      softPwmWrite(PIN_LPWM, state.LPWM);
      softPwmWrite(PIN_RPWM, state.RPWM);
    }
}

void Car::Update(CarState& newState) {
    memcpy(&state, &newState, sizeof(struct CarState));
    Write();
}

Car::~Car() {
    state.A1 = state.A2 = state.B1 = state.B2 = state.LPWM = state.RPWM = 0;
    Write();
}
