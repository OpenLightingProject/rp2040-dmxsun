#ifndef WIRELESS_H
#define WIRELESS_H

#include "pins.h"

#ifdef __cplusplus

#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>

#include "edp.h"
#include "boardconfig.h"

#include "snappy.h"

// nRF24L01+ can tune to 128 channels with 1 MHz spacing from 2400MHz to 2527MHz
#define MAXCHANNEL 128

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

    Edp edpTX;
    Edp edpRX;

    static uint8_t tmpBuf_RX0[600];
    static uint8_t tmpBuf_RX1[600];
    static uint8_t tmpBufQueueCopy[600];
    static uint8_t tmpBuf_TX1[600];

    void handleReceivedData();
    void doSendData();

    Patching findPatching(uint8_t universeId);

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
