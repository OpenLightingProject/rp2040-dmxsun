#include "dmxbuffer.h"

#include "boardconfig.h"

extern BoardConfig boardConfig;

uint8_t DmxBuffer::buffer[DMXBUFFER_COUNT][512];

void DmxBuffer::init() {
    // Simply zero out the area for the data
    memset(this->buffer, 0x00, DMXBUFFER_COUNT * 512);
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

    // TODO: Trigger patchings, wireless and sACN

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