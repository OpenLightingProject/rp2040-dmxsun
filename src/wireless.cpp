#include "wireless.h"

#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>

#include "log.h"

#include "statusleds.h"
#include "boardconfig.h"
#include "dmxbuffer.h"

extern StatusLeds statusLeds;
extern BoardConfig boardConfig;
extern DmxBuffer dmxBuffer;

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
        sleep_ms(100);
        result = rf24radio.begin(&spi);
    }

    if (!result) {
        statusLeds.setLed(6, 0, 0, 0);
        statusLeds.writeLeds();
        return;
    }

    moduleAvailable = true;

    statusLeds.setLed(6, 255, 0, 0);
    if (!rf24radio.isPVariant()) {
        statusLeds.setLed(6, 255, 255, 255);
    }
    statusLeds.writeLeds();

    // Depending on radioRole, more setup is required
    if (boardConfig.activeConfig->radioRole == RadioRole::broadcast) {
        rf24radio.setPALevel(RF24_PA_LOW);
        rf24radio.setChannel(boardConfig.activeConfig->radioChannel);
        rf24radio.enableDynamicPayloads();
        rf24radio.enableAckPayload();
        rf24radio.openWritingPipe((const uint8_t *)"DMXTX");
        rf24radio.openReadingPipe(1, (const uint8_t *)"DMXTX");
        rf24radio.startListening();
    } else if (boardConfig.activeConfig->radioRole == RadioRole::mesh) {
        rf24mesh.setNodeID(boardConfig.activeConfig->radioAddress);
        rf24mesh.begin();
    }
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
        case RadioRole::broadcast:
            // Nothing to do to keep any network alive
            this->doSendData();
            this->handleReceivedData();
            break;
        case RadioRole::mesh:
            rf24mesh.update();
            if (boardConfig.activeConfig->radioAddress == 0) {
                // We are the master of the mesh
                rf24mesh.DHCP();

                if (rf24mesh.addrListTop) {
                    statusLeds.setLed(6, 0, 255, 0);
                    statusLeds.writeLeds();
                } else {
                    statusLeds.setLed(6, 255, 0, 0);
                    statusLeds.writeLeds();
                }
            }
            this->doSendData();
            break;
    }
}

void Wireless::scanChannel(uint8_t channel)
{
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

void Wireless::sendData(uint8_t universeId, uint8_t *source, uint16_t sourceLength) {
    return;
    // This should actually only queue stuff for sending. Actual sending is done
    // in cyclicTask to avoid timeouts on the USB interface, for example
    LOG("SendData. Universe: %d. RadioRole: %d", universeId, boardConfig.activeConfig->radioRole);

    // TODO: clamp to max 512!
    memcpy(this->sendQueueData[universeId], source, sourceLength);
    this->sendQueueValid[universeId] = true;
}

void Wireless::doSendData() {
    bool success = false;

    for (int i = 0; i < 4; i++) {
        if (this->sendQueueValid[i]) {
            this->sendQueueValid[i] = false;

            switch (boardConfig.activeConfig->radioRole) {
                case RadioRole::broadcast:
                    rf24radio.stopListening();
                    success = rf24radio.write(this->sendQueueData[i], 3);
                    LOG("doSendData DONE. Success: %d", success);
                    if (success) {
                        statusLeds.setLed(6, 0, 255, 0);
                        statusLeds.writeLeds();
                    } else {
                        statusLeds.setLed(6, 255, 0, 0);
                        statusLeds.writeLeds();
                    }
                    rf24radio.startListening();
                break;
                case RadioRole::mesh:
                    if (boardConfig.activeConfig->radioAddress) {
                        // We are a node and send to the master?
                    } else {
                        // We are the mesh master and iterate through the nodes
                    }
                break;
            }
        }
    }
}

void Wireless::handleReceivedData() {
    uint8_t pipe = 0;
    uint8_t buffer[512] = {0};

    if (rf24radio.available(&pipe)) {                    // is there a payload? get the pipe number that recieved it
        uint8_t bytes = rf24radio.getDynamicPayloadSize(); // get the size of the payload

        rf24radio.read(buffer, 3);       // get incoming payload
        rf24radio.writeAckPayload(0, &buffer, 0);

        dmxBuffer.setBuffer(0, buffer, 3);
    }
}