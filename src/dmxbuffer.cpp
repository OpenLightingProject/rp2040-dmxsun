#include "dmxbuffer.h"

#include "log.h"
#include "boardconfig.h"
#include "localdmx.h"
#include "wireless.h"

extern BoardConfig boardConfig;
extern LocalDmx localDmx;
extern Wireless wireless;

extern critical_section_t bufferLock;

uint8_t DmxBuffer::buffer[DMXBUFFER_COUNT][512];
uint8_t DmxBuffer::allZeroes[512];

void DmxBuffer::init() {
    // Init the complete area to 0
    memset(this->buffer, 0x00, DMXBUFFER_COUNT * 512);

    // Init the allZeroes array
    memset(this->allZeroes, 0x00, 512);
}

void DmxBuffer::zero(uint8_t bufferId) {
    LOG("ZERO buffer %u", bufferId);

    // Simply zero out the specified buffer
    critical_section_enter_blocking(&bufferLock);
    memset(this->buffer[bufferId], 0x00, 512);
    critical_section_exit(&bufferLock);

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

    uint16_t length = MIN(sourceLength, 512);

    LOG("setBuffer Length: %d, Content: %02x %02x %02x %02x %02x %02x", sourceLength, source[0], source[1], source[2], source[3], source[4], source[5]);

    critical_section_enter_blocking(&bufferLock);
    memset(this->buffer[bufferId], 0x00, 512);
    memcpy(this->buffer[bufferId], source, length);
    critical_section_exit(&bufferLock);

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

    this->triggerPatchings(bufferId);

    return true;
}

void DmxBuffer::triggerPatchings(uint8_t bufferId, bool allZero) {
    if ((allZero) || (!memcmp(DmxBuffer::buffer[bufferId], allZeroes, 512))) {
        // universe is all zeroes
        DmxBuffer::allZeroBuffers[bufferId] = true;
    } else {
        DmxBuffer::allZeroBuffers[bufferId] = false;
    }

    LOG("DmxBuffer::triggerPatchings. bufferId: %d, allZeroes: %d", bufferId, DmxBuffer::allZeroBuffers[bufferId]);

    for (uint8_t i = 0; i < MAX_PATCHINGS; i++) {
        Patching patching = boardConfig.activeConfig->patching[i];
        if ((!patching.active) ||
            (patching.srcType != PatchType::buffer) ||
            (patching.srcInstance != bufferId))
        {
            continue;
        }

        // patching is active, matches requested bufferId and goes FROM BUFFER

        switch (patching.dstType) {
            case PatchType::local:
                localDmx.setPort(patching.dstInstance, DmxBuffer::buffer[bufferId], 512);
                LOG("DmxBuffer::triggerPatchings. Setting localDmx port %d", patching.dstInstance);
                break;
            case PatchType::nrf24:
                wireless.sendData(patching.dstInstance, DmxBuffer::buffer[bufferId], 512);
                break;
        }
    }
}
