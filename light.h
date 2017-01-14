// Enables serial printing of debug logs. Comment to disable:
#define DEBUG 1

#ifdef DEBUG
 #define DEBUG_PRINT(x)  Serial.println (x)
#else
 #define DEBUG_PRINT(x)
#endif

#ifndef LIGHT_H
#define LIGHT_H
#include <WString.h> // required for getDescription

class Light  {
  private:
    int pin;
    bool inverse;
    bool dimmable;
    int brightness;
    String description;
  public:
    Light(int light_pin, String desc, bool is_dimmable = false, bool is_inverse = false);
    void set(int new_brightness);
    int get();
    bool isDimmable();
    String getDescription();
};

#endif
