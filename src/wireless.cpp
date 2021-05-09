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

}
