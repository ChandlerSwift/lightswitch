#ifndef LIGHT_H
#define LIGHT_H

class Light  {
  private:
    bool status;
    int pin;
    bool inverse;
    bool dimmable;
    int brightness;
  public:
    Light(int light_pin, bool is_dimmable = false, bool is_inverse = false);
    void set(bool is_on);
    void set(int new_brightness);
    void on();
    void off();
    bool get();
    bool isDimmable();
};

#endif
