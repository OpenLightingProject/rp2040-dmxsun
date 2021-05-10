#include "wireless.h"

#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>

#include "statusleds.h"
#include "boardconfig.h"

extern StatusLeds statusLeds;
extern BoardConfig boardConfig;

RF24 rf24radio(PIN_RF24_CE, PIN_SPI_CS0);
RF24Network rf24network(rf24radio);
RF24Mesh rf24mesh(rf24radio, rf24network);

void Wireless::init() {
    SPI spi;
    int i = 0;

    memset(signalStrength, 0x00, MAXCHANNEL * sizeof(uint16_t));

    spi.begin(spi0);
    bool result = rf24radio.begin(&spi);
    while (!result) {
        i++;
        if (i > 5) {
            break;
        }
        sleep_ms(5000);
        result = rf24radio.begin(&spi);
    }

    if (!result) {
        statusLeds.setLed(6, 0, 0, 0);
        statusLeds.writeLeds();
        return;
    }

    statusLeds.setLed(6, 255, 0, 0);
    if (!rf24radio.isPVariant()) {
        statusLeds.setLed(6, 255, 255, 255);
    }
    statusLeds.writeLeds();

}

void Wireless::cyclicTask() {
    switch (boardConfig.activeConfig->radioRole) {
        case RadioRole::sniffer:
            lastScannedChannel++;
            if (lastScannedChannel >= MAXCHANNEL) {
                lastScannedChannel = 0;
            }
            scanChannel(lastScannedChannel);
            break;
    }
}

void Wireless::scanChannel(uint8_t channel) {
    rf24radio.setChannel(channel);
    sleep_us(130); // Let the radio tune
    rf24radio.startListening();
    sleep_us(200); // Listen for some time
    // TODO: Sleep time should ne random between 150 and 230 to
    //       avoid strobe effects from FHSS devices
    if (rf24radio.testRPD()) { // signal detected so increase signalStrength unless already maxed out
      signalStrength[channel] += (0x7FFF - signalStrength[channel]) >> 5; // increase rapidly when previous value was low, with increase reducing exponentially as value approaches maximum
    } else { // no signal detected so reduce signalStrength unless already at minimum
      signalStrength[channel] -= signalStrength[channel] >> 5; // decrease rapidly when previous value was high, with decrease reducing exponentially as value approaches zero
    }
    rf24radio.stopListening();
}