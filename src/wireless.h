#ifndef WIRELESS_H
#define WIRELESS_H

#ifdef __cplusplus

class Wireless {
  public:
    void init();
    void cyclicTask();

  private:

};

#endif // __cplusplus

// Helper methods which are called from C code
#ifdef __cplusplus
extern "C" {
#endif

// None yet :)

#ifdef __cplusplus
}
#endif

#endif // WIRELESS_H
