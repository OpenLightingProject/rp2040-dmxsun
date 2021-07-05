#include "dmxbuffer.h"

#include "boardconfig.h"
#include "localdmx.h"

extern BoardConfig boardConfig;
extern LocalDmx localDmx;
extern uint8_t allZeroes[512];

uint8_t DmxBuffer::buffer[DMXBUFFER_COUNT][512];

void DmxBuffer::init() {
    // Init the complete area to 0
    memset(this->buffer, 0x00, DMXBUFFER_COUNT * 512);
}

void DmxBuffer::zero(uint8_t bufferId) {
    // Simply zero out the specified buffer
    memset(this->buffer[bufferId], 0x00, 512);

    this->triggerPatchings(bufferId, true);
}

bool DmxBuffer::getBuffer(uint8_t bufferId, uint8_t* dest, uint16_t destLength) {
    if ((bufferId >= DMXBUFFER_COUNT) || (dest == nullptr) || destLength == 0) {
        return false;
    }
    // Shall we lock that buffer to avoid that it changes while we copy it away?

    // TODO: We don't limit destLength to <= 512. This can also be used as
    //       a feature to copy several buffers at once. Good or bad?
    memcpy(dest, this->buffer[bufferId], destLength);

    return true;
}

bool DmxBuffer::setBuffer(uint8_t bufferId, uint8_t* source, uint16_t sourceLength) {
    if ((bufferId >= DMXBUFFER_COUNT) || (source == nullptr) || sourceLength == 0) {
        return false;
    }
    // Shall we lock the buffer so two sources don't write at the same time?
    // TODO: Merge modes. For HTP and LTP we might need to remember the source that last wrote here?

    uint8_t length = MAX(sourceLength, 512);

    memset(this->buffer[bufferId], 0x00, 512);
    memcpy(this->buffer[bufferId], source, length);

    this->triggerPatchings(bufferId);

    return true;
}

bool DmxBuffer::getChannel(uint8_t bufferId, uint16_t channel, uint8_t* value) {
    if ((bufferId >= DMXBUFFER_COUNT) || (channel >= 512) || (value == nullptr)) {
        return false;
    }
    // Shall we lock that buffer to avoid that it changes while we copy it away?

    *value = this->buffer[bufferId][channel];

    return true;
}

bool DmxBuffer::setChannel(uint8_t bufferId, uint16_t channel, uint8_t value) {
    if ((bufferId >= DMXBUFFER_COUNT) || (channel >= 512)) {
        return false;
    }
    // Shall we lock the buffer so two sources don't write at the same time?
    // TODO: Merge modes. For HTP and LTP we might need to remember the source that last wrote here?

    this->buffer[bufferId][channel] = value;

    // TODO: Trigger patchings, wireless and sACN

    return true;
}

void DmxBuffer::triggerPatchings(uint8_t bufferId, bool allZero) {
    if ((allZero) || (!memcmp(DmxBuffer::buffer[bufferId], allZeroes, 512))) {
        // universe is all zeroes
        DmxBuffer::allZeroBuffers[bufferId] = true;
    }
    DmxBuffer::allZeroBuffers[bufferId] = false;

    // TODO: Patchings
    for (uint8_t i = 0; i < MAX_PATCHINGS; i++) {
        if ((!boardConfig.activeConfig->patching[i].active) ||
            (boardConfig.activeConfig->patching[i].buffer != bufferId) ||
            (boardConfig.activeConfig->patching[i].direction))
        {
            continue;
        }

        // patching is active, matches requested bufferId and goes FROM BUFFER

        if (boardConfig.activeConfig->patching[i].port <= 15) {
            // local DMX port
            memcpy(LocalDmx::buffer[boardConfig.activeConfig->patching[i].port], DmxBuffer::buffer[bufferId], 512);
        } else if (boardConfig.activeConfig->patching[i].port <= 23) {
            // local USB interface to host
            // TODO
        } else if (boardConfig.activeConfig->patching[i].port <= 27) {
            // Wireless INs
        } else if (boardConfig.activeConfig->patching[i].port <= 31) {
            // Wireless OUTs
            // TODO
        }
    }
}
