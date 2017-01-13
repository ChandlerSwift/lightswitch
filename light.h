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
