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

uint8_t Wireless::tmpBuf[800]; // Used to store compressed data
uint8_t Wireless::tmpBuf2[800]; // Used to store UNcompressed data

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
        rf24radio.setRetries(2, 2);
        rf24radio.startListening();
    } else if (boardConfig.activeConfig->radioRole == RadioRole::mesh) {
        LOG("RF24: Mesh setNodeID to %d", boardConfig.activeConfig->radioAddress);
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
    // This actually only queues stuff for sending later. Actual sending is done
    // in cyclicTask to avoid timeouts on the USB interface while waiting for
    // transmission to complete
    // It's not a real queue since the data for each universe is overwritten. No one
    // cares about the unsent, old data if we have new values anyway
    LOG("SendData. Universe: %d. RadioRole: %d", universeId, boardConfig.activeConfig->radioRole);

    uint16_t length = MAX(sourceLength, 512);

    memset(this->sendQueueData[universeId], 0x00, 512);
    memcpy(this->sendQueueData[universeId], source, length);
    this->sendQueueValid[universeId] = true;
}

void Wireless::doSendData() {
    bool success = false;

    Wireless::tmpBuf2[0] = WirelessCommands::DmxData;
    struct DmxData_Header* header = (struct DmxData_Header*)Wireless::tmpBuf2 + 1;

    size_t actuallyRead = 0;
    size_t actuallyWritten = 1000;
    size_t payloadSize = 0;

    int j = 0;

    for (int i = 0; i < 4; i++) {
        if (this->sendQueueValid[i]) {
            this->sendQueueValid[i] = false;

            switch (boardConfig.activeConfig->radioRole) {
                case RadioRole::broadcast:
                    rf24radio.stopListening();

                    // TODO: Move radio packet preparation to separate methods
                    //       so all radio modes can use it

                    header->universeId = i;

                    // 1. Check if the buffer to be sent is all zeroes
                    if (!memcmp(this->sendQueueData[i], DmxBuffer::allZeroes, 512)) {
                        header->compression = 0;
                        header->chunkCounter = DmxData_ChunkCounter::AllZero;

                        success = rf24radio.write(Wireless::tmpBuf2, 2);

                        LOG("doSendData: Sending allZeroes-Packet for universe %d Success: %d", i, success);
                    } else {
                        // 2. Compress the data
                        actuallyRead = 0;
                        actuallyWritten = 1000;
                        snappy::RawCompress((const char *)this->sendQueueData[i], 512, (char*)Wireless::tmpBuf, &actuallyWritten);

                        if (actuallyWritten >= 512) {
                            LOG("Compressed size: %d, sending uncompressed!", actuallyWritten);
                            header->compression = 0;
                            memcpy(Wireless::tmpBuf, this->sendQueueData[i], 512);
                            actuallyWritten = 512;
                        } else {
                            header->compression = 1;
                        }

                        // 3. Send the data (actuallyWritten bytes from Wireless::tmpBuf) in chunks
                        for (j = 0; j < 18; j++) {
                            Wireless::tmpBuf[0] = WirelessCommands::DmxData;

                            header->chunkCounter = (DmxData_ChunkCounter)j;
                            payloadSize = 30;

                            if ((j+1)*30 >= actuallyWritten) {
                                header->chunkCounter = DmxData_ChunkCounter::LastPacket;
                                payloadSize = actuallyWritten - j*30;
                            }

                            memcpy(Wireless::tmpBuf + 2, this->sendQueueData[i] + j*30, payloadSize);

                            success = rf24radio.write(Wireless::tmpBuf, payloadSize + 2);
                            sleep_us(100);

                            LOG("doSendData CHUNK actuallyWritten: %d, chunkCounter: %d, payloadSize: %d, Success: %d", actuallyWritten, header->chunkCounter, payloadSize, success);

                            if (header->chunkCounter == DmxData_ChunkCounter::LastPacket) {
                                break;
                            }
                        }
                        LOG("doSendData all chunks sent");

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

            LOG("doSendData DONE. Success: %d", success);
            if (success) {
                statusLeds.setLed(6, 0, 255, 0);
                statusLeds.writeLeds();
            } else {
                statusLeds.setLed(6, 255, 0, 0);
                statusLeds.writeLeds();
            }
        }
    }
}

void Wireless::handleReceivedData() {
    uint8_t pipe = 0;

    if (rf24radio.available(&pipe)) {                    // is there a payload? get the pipe number that received it
        uint8_t bytes = rf24radio.getDynamicPayloadSize(); // get the size of the payload

        rf24radio.read(Wireless::tmpBuf, bytes);       // get incoming payload
        rf24radio.writeAckPayload(0, Wireless::tmpBuf, 0);

        LOG("Wireless RX: %d byte. Command: %d", bytes, Wireless::tmpBuf[0]);

        if (Wireless::tmpBuf[0] == WirelessCommands::DmxData) {
            struct DmxData_Header* header = (struct DmxData_Header*)Wireless::tmpBuf + 1;

            LOG("DmxData: Universe: %d, Compressed: %d, Chunk: %d", header->universeId, header->compression, header->chunkCounter);
        }

        // TODO: Proper patching handling, please
        // Dirty workaround for RX demonstration. This will LOOP if the
        // writing to the DMX buffer will trigger a TX action
        //dmxBuffer.setBuffer(0, buffer, 3);
    }
}