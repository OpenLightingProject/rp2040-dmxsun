#ifndef WIRELESS_H
#define WIRELESS_H

#include "pins.h"

#ifdef __cplusplus

#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>

// nRF24L01+ can tune to 128 channels with 1 MHz spacing from 2400MHz to 2527MHz
#define MAXCHANNEL 128

class Wireless {
  public:
    void init();
    void cyclicTask();

    uint16_t signalStrength[MAXCHANNEL]; // Used for spectrum analyser mode

  private:
    uint8_t lastScannedChannel = 0;
    void scanChannel(uint8_t channel);

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
