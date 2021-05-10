#ifndef DMXBUFFER_H
#define DMXBUFFER_H

#include <stdio.h>

#ifndef DMXBUFFER_COUNT
#define DMXBUFFER_COUNT 24
#endif // DMXBUFFER_COUNT

#ifdef __cplusplus

class DmxBuffer {
  public:
    static uint8_t buffer[DMXBUFFER_COUNT][512];
    void init();
    bool getBuffer(uint8_t bufferId, uint8_t* dest, uint16_t destLength);
    bool setBuffer(uint8_t bufferId, uint8_t* source, uint16_t sourceLength);

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

#endif // DMXBUFFER_H
