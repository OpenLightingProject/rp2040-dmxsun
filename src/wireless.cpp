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

extern critical_section_t bufferLock;

uint16_t Wireless::packetLen;

uint8_t Wireless::tmpBuf[800]; // Used to store compressed data
uint8_t Wireless::tmpBuf2[800]; // Used to store UNcompressed data

// TODO: Do we need one tmpBuf per incoming universe to assemble them?
//       Or can we expect them to come in order?
//       Currently using tmpBuf2 as assemble-buffer for ALL INCOMING packets + outgoing will overwrite them!

RF24 rf24radio(PIN_RF24_CE, PIN_SPI_CS0);
RF24Network rf24network(rf24radio);
RF24Mesh rf24mesh(rf24radio, rf24network);

void Wireless::init() {
    SPI spi;
    int i = 0;

    packetLen = 0;

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
        rf24radio.setPALevel(RF24_PA_MAX, true);
        rf24radio.setChannel(boardConfig.activeConfig->radioChannel);
        rf24radio.setDataRate(RF24_2MBPS);
        rf24radio.enableDynamicPayloads();
        rf24radio.setAutoAck(true);
        rf24radio.setCRCLength(RF24_CRC_8);
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
    if (!moduleAvailable) {
        return;
    }

    // This actually only queues stuff for sending later. Actual sending is done
    // in cyclicTask to avoid timeouts on the USB interface while waiting for
    // transmission to complete
    // It's not a real queue since the data for each universe is overwritten. No one
    // cares about the unsent, old data if we have new values anyway
    LOG("SendData. Universe: %d. RadioRole: %d", universeId, boardConfig.activeConfig->radioRole);

    uint16_t length = MAX(sourceLength, 512);

    critical_section_enter_blocking(&bufferLock);
    memset(this->sendQueueData[universeId], 0x00, 512);
    memcpy(this->sendQueueData[universeId], source, length);
    critical_section_exit(&bufferLock);

    this->sendQueueValid[universeId] = true;
}

void Wireless::doSendData() {
    bool success = false;
    uint8_t automaticRetryCount = 0;

    Wireless::tmpBuf2[0] = WirelessCommands::DmxData;
    struct DmxData_PacketHeader* packetHeader;
    struct DmxData_ChunkHeader* chunkHeader;

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

                    // 1. Check if the buffer to be sent is all zeroes
                    //    If so, don't create an actual packet, only send one special chunk
                    if (!memcmp(this->sendQueueData[i], DmxBuffer::allZeroes, 512)) {
                        chunkHeader = (struct DmxData_ChunkHeader*)Wireless::tmpBuf2 + 1;
                        chunkHeader->universeId = i;
                        chunkHeader->chunkCounter = DmxData_ChunkCounter::AllZero;
                        chunkHeader->lastChunk = 1; // Strictly not needed

                        success = rf24radio.write(Wireless::tmpBuf2, 2);

                        LOG("doSendData: Sending allZeroes-Packet for universe %d Success: %d", i, success);
                    } else {
                        packetHeader = (struct DmxData_PacketHeader*)(Wireless::tmpBuf);

                        // 2. Compress the data
                        actuallyRead = 0;
                        actuallyWritten = 700;
                        snappy::RawCompress((const char *)this->sendQueueData[i], 512, (char*)(Wireless::tmpBuf + sizeof(struct DmxData_PacketHeader)), &actuallyWritten);

                        if (actuallyWritten >= 512) {
                            LOG("Compressed size: %d => SENDING UNCOMPRESSED!", actuallyWritten);
                            packetHeader->compressed = 0;
                            memcpy(Wireless::tmpBuf + sizeof(struct DmxData_PacketHeader), this->sendQueueData[i], 512);
                            actuallyWritten = 512;
                        } else {
                            packetHeader->compressed = 1;
                        }

                        // Increase the size of the packet by the prepended header
                        actuallyWritten += sizeof(struct DmxData_PacketHeader);

                        // Not yet supported
                        packetHeader->partial = 0;
                        packetHeader->partialOffset = 0;

                        // TODO: Calculate some kind of CRC so the receivers know if they got all chunks
                        //packetHeader->crc = XXX;

                        // 3. Send the data (actuallyWritten bytes from Wireless::tmpBuf) in chunks
                        //    Use tmpBuf2 to construct the chunk
                        for (j = 0; j < 18; j++) {
                            Wireless::tmpBuf2[0] = WirelessCommands::DmxData;
                            chunkHeader = (DmxData_ChunkHeader*)(Wireless::tmpBuf2 + 1);

                            chunkHeader->universeId = i;
                            chunkHeader->chunkCounter = (DmxData_ChunkCounter)j;
                            chunkHeader->lastChunk = 0;
                            payloadSize = 30;

                            if ((j+1)*30 >= actuallyWritten) {
                                chunkHeader->lastChunk = 1;
                                payloadSize = actuallyWritten - j*30;
                            }

                            memcpy(Wireless::tmpBuf2 + 2, Wireless::tmpBuf + j*30, payloadSize);

                            success = rf24radio.write(Wireless::tmpBuf2, payloadSize + 2);
                            automaticRetryCount = rf24radio.getARC();

                            LOG("doSendData CHUNK TotalSize: %d, chunkCounter: %d, payloadSize: %d, buf: %02x %02x %02x %02x %02x %02x %02x %02x %02x, Success: %d, Retries: %d", actuallyWritten, chunkHeader->chunkCounter, payloadSize, Wireless::tmpBuf2[0], Wireless::tmpBuf2[1], Wireless::tmpBuf2[2], Wireless::tmpBuf2[3], Wireless::tmpBuf2[4], Wireless::tmpBuf2[5], Wireless::tmpBuf2[6], Wireless::tmpBuf2[7], Wireless::tmpBuf2[8], success, automaticRetryCount);

                            // If this chunk didn't get any ACKs, don't send the following ones as well
                            // Since no one can re-assemble that packet and it just wastes our time
                            if ((chunkHeader->lastChunk) || (!success)) {
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
    size_t uncompressedLength;

    // TODO: Check for radio mode

    if (rf24radio.available(&pipe)) {                    // is there a payload? get the pipe number that received it
        uint8_t bytes = rf24radio.getDynamicPayloadSize(); // get the size of the payload

        rf24radio.read(Wireless::tmpBuf, bytes);       // get incoming payload
        // No need to manually send an ACK since autoAck is being used

        LOG("Wireless RX: %d byte. Command: %d", bytes, Wireless::tmpBuf[0]);

        if (Wireless::tmpBuf[0] == WirelessCommands::DmxData) {
            struct DmxData_ChunkHeader* chunkHeader = (struct DmxData_ChunkHeader*)Wireless::tmpBuf + 1;

            LOG("DmxData: Universe: %d, Chunk: %d, LastChunk: %d", chunkHeader->universeId, chunkHeader->chunkCounter, chunkHeader->lastChunk);

            for (int i = 0; i < MAX_PATCHINGS; i++) {
                Patching patching = boardConfig.activeConfig->patching[i];
                if ((!patching.active) ||
                    ((patching.port - 24) != chunkHeader->universeId) ||
                    (!patching.direction))
                {
                    continue;
                }

                LOG("Wireless IN found patched to buffer %d", patching.buffer);

                if (chunkHeader->chunkCounter == DmxData_ChunkCounter::AllZero) {
                    // Easy: Just clear the DmxBuffer
                    dmxBuffer.zero(patching.buffer);
                } else if (chunkHeader->chunkCounter == DmxData_ChunkCounter::FirstPacket) {
                    critical_section_enter_blocking(&bufferLock);
                    // Clear tmpBuf2 so the next chunk comes in clean
                    memset(Wireless::tmpBuf2, 0x00, 800);
                    packetLen = 0;

                    memcpy(Wireless::tmpBuf2, Wireless::tmpBuf + 2, bytes - 2);
                    packetLen += (bytes - 2);
                    critical_section_exit(&bufferLock);
                } else {
                    // Some intermediate packet: Just copy it to the tmpBuf and go ahead
                    critical_section_enter_blocking(&bufferLock);
                    memcpy(Wireless::tmpBuf2 + chunkHeader->chunkCounter*30, Wireless::tmpBuf + 2, bytes - 2);
                    packetLen += (bytes - 2);
                    critical_section_exit(&bufferLock);

                    // TODO: Remember which chunks actually came in

                    if (chunkHeader->lastChunk) {
                        LOG("LastPacket came in, assembly complete! packetLen is now %d", packetLen);

                        struct DmxData_PacketHeader* packetHeader = (struct DmxData_PacketHeader*)Wireless::tmpBuf2;

                        // TODO: Check if all chunks have arrived and check CRC

                        if (packetHeader->compressed) {
                            // If compressed, uncompress ;)
                            if (snappy::GetUncompressedLength((const char*)(Wireless::tmpBuf2 + sizeof(struct DmxData_PacketHeader)), packetLen - 4, &uncompressedLength) == true) {
                                LOG("snappy::GetUncompressedLength: %d", uncompressedLength);

                                // Sanity check: uncompressedLength must be 512
                                if (uncompressedLength != 512) {
                                    return;
                                }

                                if (snappy::RawUncompress((const char*)(Wireless::tmpBuf2 + sizeof(struct DmxData_PacketHeader)), packetLen - 4, (char*)Wireless::tmpBuf) == true) {
                                    dmxBuffer.setBuffer(patching.buffer, Wireless::tmpBuf, uncompressedLength);
                                } else {
                                    LOG("snappy::RawUncompress failed :(");
                                }
                            } else {
                                LOG("snappy::GetUncompressedLength failed :(");
                            }
                        } else {
                            // Sanity check: if full frame, packetLen MUST be 512 + sizeof PacketHeader
                            if (packetHeader->partial || packetLen == (512 + sizeof(DmxData_PacketHeader))) {
                                dmxBuffer.setBuffer(patching.buffer, Wireless::tmpBuf2 + sizeof(struct DmxData_PacketHeader), (packetLen - sizeof(struct DmxData_PacketHeader)));
                            }
                        }
                    }
                }
            }

        }
    }
}
