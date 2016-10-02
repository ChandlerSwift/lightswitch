#include "overhead_light.h"
#include "Arduino.h"

OverheadLight::OverheadLight() {
  light_pin = 5;
  indicator_pin = 2;
  pinMode(light_pin, OUTPUT);
  pinMode(indicator_pin, OUTPUT);
  set (off);
}

void OverheadLight::set(bool isOn) {
  status = isOn;
  digitalWrite(light_pin, status);
  digitalWrite(indicator_pin, !status); // inverted because grounding activates
}

bool OverheadLight::get() {
  return status;
}
