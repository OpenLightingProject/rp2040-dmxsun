#ifndef ETH_CYW43_H
#define ETH_CYW43_H

class Eth_cyw43 {
  public:
    void init();
    void cyclicTask();

  private:
};

#ifndef PICO_CYW43_SUPPORTED
    // Dummies so we don't have to do the #ifdef-dance on every call to those
    void Eth_cyw43::init() {};
    void Eth_cyw43::cyclicTask() {};
#endif

#endif // ETH_CYW43_H
