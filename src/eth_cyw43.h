#ifndef ETH_CYW43_H
#define ETH_CYW43_H

class Eth_cyw43 {
  public:
    void init();
    void cyclicTask();

  private:
};

// If we are NOT on pico_w, we have PIN_LED and we need to define dummies
// since eth_cyw43.cpp won't be compiled
#ifdef PIN_LED
    // Dummies so we don't have to do the #ifdef-dance on every call to those
    void Eth_cyw43::init() {};
    void Eth_cyw43::cyclicTask() {};
#endif

#endif // ETH_CYW43_H
