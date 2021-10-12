#include "edp.h"

#include "boardconfig.h"
#include "dmxbuffer.h"

extern BoardConfig boardConfig;
extern DmxBuffer dmxBuffer;

extern critical_section_t bufferLock;

void Edp::init(uint8_t* inData, uint8_t* outChunk, uint8_t patchingOffset, uint16_t maxSendChunkSize) {
    this->initOkay = false;

    if (!inData || !outChunk || (maxSendChunkSize < 20)) {
        return;
    }

    this->inData = inData;
    this->outChunk = outChunk;
    this->patchingOffset = patchingOffset;
    this->maxSendChunkSize = maxSendChunkSize;

    this->initOkay = true;
}

// Take data from inData, prepare the complete packet in scratch
// Then, chop it into chunks and store them in outChunk, one per call
bool Edp::prepareDmxData(uint8_t universeId, uint16_t inDataSize, bool allZero, uint16_t* thisChunkSize, bool* callAgain) {
    uint16_t limitedInDataSize;
    uint8_t* destination;

    struct Edp_DmxData_ChunkHeader* chunkHeader = (struct Edp_DmxData_ChunkHeader*)(outChunk + sizeof(Edp_Commands));
    struct Edp_DmxData_PacketHeader* packetHeader = (struct Edp_DmxData_PacketHeader*)(outChunk + sizeof(Edp_Commands) + sizeof(Edp_DmxData_ChunkHeader));

    if (inDataSize != 0) {
        // Start a new packet, discard existing data and chunks

        memset(outChunk, 0x00, 600);
        outChunk[0] = Edp_Commands::DmxData;
        packetHeader->universeId = universeId;

        // Special case: allZero packet
        if (allZero) {
            chunkHeader->chunkCounter = Edp_DmxData_ChunkCounter::AllZero;
            chunkHeader->lastChunk = true; // Actually not needed
            *thisChunkSize = sizeof(Edp_Commands) + sizeof(struct Edp_DmxData_ChunkHeader) + sizeof(struct Edp_DmxData_PacketHeader);
            *callAgain = false;
            return true;
        }

        limitedInDataSize = MIN(inDataSize, 512);

        prepareDmxData_chunkOffset = maxSendChunkSize;

        // TODO: Not yet supported
        packetHeader->partial = 0;
        packetHeader->partialOffset = 0;

        // Compress inData to outChunk. If it's larger than the input, it will be overwritten later
        prepareDmxData_sizeOfDataToBeSent = 600 - sizeof(Edp_Commands) - sizeof(Edp_DmxData_ChunkHeader) - sizeof(Edp_DmxData_PacketHeader);
        destination = outChunk + sizeof(Edp_Commands) + sizeof(struct Edp_DmxData_ChunkHeader) + sizeof(Edp_DmxData_PacketHeader);
        snappy::RawCompress((const char *)inData, limitedInDataSize, (char*)destination, &prepareDmxData_sizeOfDataToBeSent);

        if (prepareDmxData_sizeOfDataToBeSent >= limitedInDataSize) {
            LOG("Compressed size: %d (inSize: %d) => SENDING UNCOMPRESSED!", prepareDmxData_sizeOfDataToBeSent, limitedInDataSize);
            packetHeader->compressed = 0;
            memcpy(destination, inData, limitedInDataSize);
            prepareDmxData_sizeOfDataToBeSent = limitedInDataSize;
        } else {
            packetHeader->compressed = 1;
        }

        // Increase the size of the packet by the prepended header
        prepareDmxData_sizeOfDataToBeSent += sizeof(struct Edp_DmxData_PacketHeader);

        // TODO: Calculate some kind of CRC so the receivers know if they got all chunks
        //packetHeader->crc = XXX;

        // Make chunk 0 ready
        chunkHeader->chunkCounter = Edp_DmxData_ChunkCounter::FirstPacket;
        if (prepareDmxData_sizeOfDataToBeSent <= maxSendChunkSize) {
            // Yay, only one chunk needed :D
            chunkHeader->lastChunk = true;
            *thisChunkSize = prepareDmxData_sizeOfDataToBeSent;
            *callAgain = false;
            return true;
        }

        chunkHeader->lastChunk = false;

        *thisChunkSize = maxSendChunkSize;
        *callAgain = true;
        return true;

    } else {
        // Next chunk if available, otherwise return false
        // TODO: Check if there is a next chunk

        // ChunkOffset points to the OLD chunk's data

        destination = outChunk + sizeof(Edp_Commands) + sizeof(struct Edp_DmxData_ChunkHeader);
        memcpy(destination, outChunk + prepareDmxData_chunkOffset, maxSendChunkSize - sizeof(Edp_Commands) - sizeof(struct Edp_DmxData_ChunkHeader));

        chunkHeader->chunkCounter = (Edp_DmxData_ChunkCounter)(chunkHeader->chunkCounter + 1);

        if (prepareDmxData_chunkOffset + maxSendChunkSize >= prepareDmxData_sizeOfDataToBeSent) {
            chunkHeader->lastChunk = true;
            *thisChunkSize = prepareDmxData_sizeOfDataToBeSent - prepareDmxData_chunkOffset + sizeof(struct Edp_DmxData_PacketHeader);
            *callAgain = false;
            return true;
        }

        prepareDmxData_chunkOffset = prepareDmxData_chunkOffset + (maxSendChunkSize - sizeof(Edp_Commands) - sizeof(Edp_DmxData_ChunkHeader));
        *callAgain = true;

        return true;
    }
}

bool Edp::processIncomingChunk(uint8_t* chunkData, uint16_t chunkSize) {

}