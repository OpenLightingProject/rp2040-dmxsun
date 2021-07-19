#ifndef WIRELESS_H
#define WIRELESS_H

#include "pins.h"

#ifdef __cplusplus

#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>

// nRF24L01+ can tune to 128 channels with 1 MHz spacing from 2400MHz to 2527MHz
#define MAXCHANNEL 128

// DATA TYPES

// TODO: Wireless protocol for "broadcast" mode

// 32 byte RF24 payload MAX
// 1 byte COMMAND
// 1 byte "SendDmx" header
//     = 30 byte DMX data per packet. 512/30 = 18 packets MAX => 5 bit for the chunk counter

class Wireless {
  public:
    void init();
    void cyclicTask();

    bool moduleAvailable = false;
    uint16_t signalStrength[MAXCHANNEL]; // Used for spectrum analyser mode

    void sendData(uint8_t universeId, uint8_t* source, uint16_t sourceLength);

    // TODO: Function to get/set the whole set or single parameters
    //       such as role, channel, txPower

    // TODO: Function to get mesh status and nodes

  private:
    uint8_t lastScannedChannel = 0;
    void scanChannel(uint8_t channel);
    bool sendQueueValid[4];
    uint8_t sendQueueData[4][512];

    void handleReceivedData();
    void doSendData();

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
