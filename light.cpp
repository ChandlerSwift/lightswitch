#include "light.h"
#include "Arduino.h"

Light::Light (int light_pin, String desc, bool is_dimmable, bool is_inverse) : 
    pin(light_pin), description(desc), dimmable(is_dimmable), inverse(is_inverse)  {
  Serial.print("Setting up light ");
  Serial.print(light_pin);
  if (is_inverse) {
    Serial.print(" (inverted)");
  }
  Serial.println(".");
  pinMode(pin, OUTPUT);
  set(0);
}

void Light::set(int new_brightness) {
  Serial.println(new_brightness);
  if (dimmable) {
    // bounds checking
    if (new_brightness < 0) new_brightness = 0;
    if (new_brightness > 255) new_brightness = 255;
  } else {
    // all nonzero are on
    if (new_brightness != 0) new_brightness = 255;
  }
  brightness = new_brightness;
  if (inverse) new_brightness = 255 - new_brightness;
  Serial.print("Setting light on pin ");
  Serial.print(pin);
  Serial.print(" to ");
  Serial.println(brightness);
  analogWrite(pin, new_brightness);
}

int Light::get() {
  return dimmable ? brightness : brightness/255;
}

bool Light::isDimmable() {
  return dimmable;
}

String Light::getDescription() {
  return description;
}

