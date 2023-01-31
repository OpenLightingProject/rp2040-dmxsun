#include "wireless.h"

#include <RF24.h>
#include <RF24Network.h>
#include <RF24Mesh.h>

#include "json/json.h"

#include "log.h"

#include "statusleds.h"
#include "boardconfig.h"
#include "dmxbuffer.h"

extern StatusLeds statusLeds;
extern BoardConfig boardConfig;
extern DmxBuffer dmxBuffer;

extern critical_section_t bufferLock;

uint8_t Wireless::tmpBuf_RX0[600]; // Used to store incoming data from radio
uint8_t Wireless::tmpBuf_RX1[600]; // Used by edpRX to assemble the packets
uint8_t Wireless::tmpBufQueueCopy[600]; // Used to quickly copy data from the sendQueue. goes to edpTX as inData
uint8_t Wireless::tmpBuf_TX1[600]; // Used by edpRX to store the chunks ready to be sent

// TODO: Do we need one tmpBuf per incoming universe to assemble them?
//       Or can we expect them to come in order?
//       Currently using tmpBuf2 as assemble-buffer for ALL INCOMING packets + outgoing will overwrite them!

RF24 rf24radio(PIN_RF24_CE, PIN_SPI_CS0);
RF24Network rf24network(rf24radio);
RF24Mesh rf24mesh(rf24radio, rf24network);

void Wireless::init() {
    SPI spi;
    int i = 0;

    // Stats
    memset(&stats, 0x00, sizeof(struct WirelessStats));

    // RX path goes via RX0 from radio to EDP and RX1 is out buffer
    edpRX.init(tmpBuf_RX0, tmpBuf_RX1, 32, PatchType::nrf24);

    // TX path goes from sendQueueCopy to EDP and TX1 it out buffer
    edpTX.init(tmpBufQueueCopy, tmpBuf_TX1, 32, PatchType::nrf24);

    memset(signalStrength, 0x00, MAXCHANNEL * sizeof(uint16_t));

    spi.begin(spi0);

    bool result = rf24radio.begin(&spi);
    while (!result) {
        i++;
        if (i > 5) {
            break;
        }
        sleep_ms(50);
        result = rf24radio.begin(&spi);
    }

    if (!result) {
        statusLeds.setStatic(6, 0, 0, 0);
        return;
    }

    moduleAvailable = true;

    statusLeds.setStatic(6, 1, 0, 0);
    if (!rf24radio.isPVariant()) {
        statusLeds.setStatic(6, 1, 1, 1);
    }
    statusLeds.writeLeds();

    // Depending on radioRole, more setup is required
    if (boardConfig.activeConfig->radioRole == RadioRole::broadcast) {
        rf24radio.setPALevel(boardConfig.activeConfig->radioParams.txPower, true);
        rf24radio.setChannel(boardConfig.activeConfig->radioChannel);
        rf24radio.setDataRate(boardConfig.activeConfig->radioParams.dataRate);
        rf24radio.enableDynamicPayloads();
        rf24radio.setAutoAck(true);
        rf24radio.setCRCLength(RF24_CRC_16);
        rf24radio.disableAckPayload();
        rf24radio.openWritingPipe((const uint8_t *)"DMXTX");
        rf24radio.openReadingPipe(1, (const uint8_t *)"DMXTX");
        rf24radio.setRetries(0, 8);
        rf24radio.startListening();
    } else if (boardConfig.activeConfig->radioRole == RadioRole::mesh) {
        LOG("RF24: Mesh setNodeID to %d", boardConfig.activeConfig->radioAddress);
        rf24mesh.setNodeID(boardConfig.activeConfig->radioAddress);
        rf24mesh.begin();
    }
}

void Wireless::cyclicTask() {
    if (!moduleAvailable) {
        return;
    }

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
                    statusLeds.setStatic(6, 0, 1, 0);
                } else {
                    statusLeds.setStatic(6, 1, 0, 0);
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
    if (!moduleAvailable) {
        return;
    }

    // This actually only queues stuff for sending later. Actual sending is done
    // in cyclicTask to avoid timeouts on the USB interface while waiting for
    // transmission to complete
    // It's not a real queue since the data for each universe is overwritten. No one
    // cares about the unsent, old data if we have new values anyway
    LOG("SendData. Universe: %d. RadioRole: %d", universeId, boardConfig.activeConfig->radioRole);

    uint16_t length = MIN(sourceLength, 512);

    // TODO: Mutex should be fine here, no reason to disables IRQs
    critical_section_enter_blocking(&bufferLock);
    memset(this->sendQueueData[universeId], 0x00, 512);
    memcpy(this->sendQueueData[universeId], source, length);
    critical_section_exit(&bufferLock);

    this->sendQueueValid[universeId] = true;
}

void Wireless::doSendData() {
    bool triedToSend = false;
    bool success = false;
    bool anyFailed = false;
    uint8_t automaticRetryCount = 0;

    uint16_t thisChunkSize = 0;
    bool callAgain = false;

    size_t actuallyWritten = 1000;
    size_t payloadSize = 0;

    int j = 0;

    for (int i = 0; i < 4; i++) {
        if (this->sendQueueValid[i]) {

            critical_section_enter_blocking(&bufferLock);
            this->sendQueueValid[i] = false;

            // Copy the data away to somewhere it doesn't change while we read it
            memcpy(Wireless::tmpBufQueueCopy, this->sendQueueData[i], 512);
            critical_section_exit(&bufferLock);

            triedToSend = true;
            statusLeds.setBlinkOnce(6, 0, 1, 0);

            switch (boardConfig.activeConfig->radioRole) {
                case RadioRole::broadcast:
                    rf24radio.stopListening();

                    callAgain = false;
                    edpTX.prepareDmxData(i, 512, &thisChunkSize, &callAgain);
                    stats.sentTried++;
                    success = rf24radio.write(Wireless::tmpBuf_TX1, thisChunkSize);
                    if (!success) {
                        anyFailed = true;
                    } else {
                        stats.sentSuccess++;
                    }
                    while(callAgain) {
                        edpTX.prepareDmxData(i, 0, &thisChunkSize, &callAgain);
                        success = rf24radio.write(Wireless::tmpBuf_TX1, thisChunkSize);
                        stats.sentTried++;
                        if (!success) {
                            anyFailed = true;
                        } else {
                            stats.sentSuccess++;
                        }
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

            LOG("doSendData DONE");
        }
    }

    if (triedToSend) {
        if (anyFailed) {
            statusLeds.setStaticOn(6, 1, 0, 0);
        } else {
            statusLeds.setStaticOff(6, 1, 0, 0);
        }
    }
}

void Wireless::handleReceivedData() {
    size_t copySize = 0;
    uint8_t pipe = 0;
    size_t uncompressedLength;
    Patching patching;

    // TODO: Check for radio mode

    if (rf24radio.available(&pipe)) {                    // is there a payload? get the pipe number that received it
        uint8_t bytes = rf24radio.getDynamicPayloadSize(); // get the size of the payload

        memset(Wireless::tmpBuf_RX0, 0x00, 32); // make sure we have defined values as long as one chunk can be
        rf24radio.read(Wireless::tmpBuf_RX0, bytes);       // get incoming payload
        // No need to manually send an ACK since autoAck is being used

        if (bytes < 1) {
            // TODO: Does it make sense to have "command only" commands?
            //       If not, we should fail if bytes < 2
            return;
        }

        stats.received++;

        statusLeds.setBlinkOnce(6, 0, 0, 1);

        edpRX.processIncomingChunk(bytes);
    }
}

std::string Wireless::getWirelessStats() {
    Json::Value output;
    Json::StreamWriterBuilder wbuilder;
    std::string output_string;

    wbuilder["indentation"] = "";

    output["sentTried"] = stats.sentTried;
    output["sentSuccess"] = stats.sentSuccess;
    output["received"] = stats.received;
    output_string = Json::writeString(wbuilder, output);
    return output_string;
}
