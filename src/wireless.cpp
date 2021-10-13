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
uint8_t Wireless::tmpBufQueueCopy[600]; // Used to quickly copy data from the sendQueue

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

    edp.init(Wireless::tmpBufQueueCopy, Wireless::tmpBuf, 24, 32);

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

    Wireless::tmpBuf2[0] = WirelessCommands::WL_DmxData;
    struct DmxData_PacketHeader* packetHeader;
    struct DmxData_ChunkHeader* chunkHeader;

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
                    edp.prepareDmxData(i, 512, &thisChunkSize, &callAgain);
                    success = rf24radio.write(Wireless::tmpBuf, thisChunkSize);
                    if (!success) {
                        anyFailed = true;
                    }
                    while(callAgain) {
                        edp.prepareDmxData(i, 0, &thisChunkSize, &callAgain);
                        success = rf24radio.write(Wireless::tmpBuf, thisChunkSize);
                        if (!success) {
                            anyFailed = true;
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

Patching Wireless::findPatching(uint8_t universeId) {
    Patching retPatch;

    retPatch.active = false;

    for (int i = 0; i < MAX_PATCHINGS; i++) {
        Patching patching = boardConfig.activeConfig->patching[i];
        if ((!patching.active) ||
            ((patching.port - 24) != universeId) ||
            (!patching.direction))
        {
            continue;
        }
        return patching;
    }

    return retPatch;
}

void Wireless::handleReceivedData() {
    size_t copySize = 0;
    uint8_t pipe = 0;
    size_t uncompressedLength;

    // TODO: Check for radio mode

    if (rf24radio.available(&pipe)) {                    // is there a payload? get the pipe number that received it
        uint8_t bytes = rf24radio.getDynamicPayloadSize(); // get the size of the payload

        memset(Wireless::tmpBuf, 0x00, 32); // make sure we have defined values as long as one chunk can be
        rf24radio.read(Wireless::tmpBuf, bytes);       // get incoming payload
        // No need to manually send an ACK since autoAck is being used

        if (bytes < 1) {
            // TODO: Does it make sense to have "command only" commands?
            //       If not, we should fail if bytes < 2
            return;
        }

        statusLeds.setBlinkOnce(6, 0, 0, 1);

        // TODO: START refactor to EDP classes

        LOG("Wireless RX: %d byte. Command: %d", bytes, Wireless::tmpBuf[0]);

        if (Wireless::tmpBuf[0] == WirelessCommands::WL_DmxData) {

            if (bytes < 2) {
                return;
            }

            struct DmxData_ChunkHeader* chunkHeader = (struct DmxData_ChunkHeader*)Wireless::tmpBuf + 1;

            LOG("DmxData: Chunk: %d, LastChunk: %d", chunkHeader->chunkCounter, chunkHeader->lastChunk);

            // TODO: Instead of looping through the patchings first and then
            //       assembling the packet, we should better assemble first
            //       and then loop through the patchings when we got the
            //       complete frame assembled

            // TODO: Verify what happens if multiple patchings match the packet!

            //LOG("Wireless IN found patched to buffer %d", patching.buffer);

            // Current packet's data (one chunk) is in tmpBuf
            // Complete frame (all chunks) is assembled in tmpBuf2

            Patching patching;

            if (chunkHeader->chunkCounter == DmxData_ChunkCounter::WL_AllZero) {
                struct DmxData_PacketHeader* packetHeader = (struct DmxData_PacketHeader*)(Wireless::tmpBuf + 2);
                patching = findPatching(packetHeader->universeId);

                LOG("allZero packet. universe: %u patching active: %u buffer: %u", packetHeader->universeId, patching.active, patching.buffer);

                if (patching.active) {
                    // Easy: Just clear the DmxBuffer
                    dmxBuffer.zero(patching.buffer);
                }
            } else if (chunkHeader->chunkCounter == DmxData_ChunkCounter::WL_FirstPacket) {
                // Clear tmpBuf2 so the next chunk comes in clean
                packetLen = MIN((bytes - 2), 32);
                critical_section_enter_blocking(&bufferLock);
                memset(Wireless::tmpBuf2, 0x00, 800);
                memcpy(Wireless::tmpBuf2, Wireless::tmpBuf + 2, packetLen);
                critical_section_exit(&bufferLock);
            } else if (chunkHeader->chunkCounter < 20) {
                // Some intermediate packet: Just copy it to the tmpBuf and go ahead
                copySize = MIN((bytes - 2), 32);
                critical_section_enter_blocking(&bufferLock);
                memcpy(Wireless::tmpBuf2 + chunkHeader->chunkCounter*30, Wireless::tmpBuf + 2, copySize);
                critical_section_exit(&bufferLock);
                packetLen += copySize;

                // TODO: Remember which chunks actually came in

                if (chunkHeader->lastChunk) {
                    struct DmxData_PacketHeader* packetHeader = (struct DmxData_PacketHeader*)Wireless::tmpBuf2;

                    patching = findPatching(packetHeader->universeId);

                    if (!patching.active) {
                        return;
                    }

                    LOG("LastPacket came in, assembly complete! packetLen is now %d. Compressed: %u Universe: %u", packetLen, packetHeader->compressed, packetHeader->universeId);

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

        // END refactor to EDP

    }
}
