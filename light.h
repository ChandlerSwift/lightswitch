#ifndef LIGHT_H
#define LIGHT_H

class Light  {
  private:
    int pin;
    bool inverse;
    bool dimmable;
    int brightness;
  public:
    Light(int light_pin, bool is_dimmable = false, bool is_inverse = false);
    void set(int new_brightness);
    int get();
    bool isDimmable();
};

#endif
