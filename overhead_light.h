#ifndef OVERHEAD_LIGHT_H
#define OVERHEAD_LIGHT_H

class OverheadLight {
  private:
    bool status;
    int light_pin;
    int indicator_pin;
  public:
    static const bool on = true;
    static const bool off = false;
    OverheadLight();
    void set(bool isOn);
    bool get();
};

#endif
