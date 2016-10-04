#include "light.h"
#include "Arduino.h"

Light::Light (int light_pin, bool is_dimmable, bool is_inverse) : pin(light_pin), dimmable(is_dimmable), inverse(is_inverse)  {
  Serial.print("Setting up light ");
  Serial.print(light_pin);
  if (is_inverse) {
    Serial.print(" (inverted)");
  }
  Serial.println(".");
  pinMode(pin, OUTPUT);
  set(false);
}

void Light::set(bool is_on) {
  Serial.print(String("Turning ") + (is_on ? "on" : "off") + " light ");
  Serial.print(pin);
  Serial.println(".");
  status = is_on;
  digitalWrite(pin, inverse != status);
}

void Light::set(int new_brightness) {
  brightness = new_brightness;
  //digitalWrite(pin, inverse != status);
}

void Light::on() {
  status = true;
  digitalWrite(pin, !inverse);
}
void Light::off() {
  status = false;
  digitalWrite(pin, inverse);
}

bool Light::get() {
  return status;
}

bool Light::isDimmable() {
  return dimmable;
}

