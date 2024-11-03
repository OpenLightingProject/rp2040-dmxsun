#include "edp.h"

#include "crc_X25.h"

#include "boardconfig.h"
#include "dmxbuffer.h"

extern BoardConfig boardConfig;
extern DmxBuffer dmxBuffer;

extern critical_section_t bufferLock;

void Edp::init(uint8_t* inData, uint8_t* outData, uint16_t maxSendChunkSize, PatchType patchSource) {
    this->initOkay = false;

    if (!inData || !outData || (maxSendChunkSize < 20)) {
        return;
    }

    this->inData = inData;
    this->outData = outData;
    this->maxSendChunkSize = maxSendChunkSize;
    this->patchSource = patchSource;

    this->initOkay = true;
}

// Take data from inData, prepare the complete packet in scratch
// Then, chop it into chunks and store them in outData, one per call
bool Edp::prepareDmxData(uint8_t universeId, uint16_t inDataSize, uint16_t* thisChunkSize, bool* callAgain) {
    uint16_t limitedInDataSize;
    uint16_t sparseSize;
    uint8_t* destination;
    uint16_t firstUsedChannel;
    uint16_t lastUsedChannel;

    struct Edp_DmxData_ChunkHeader* chunkHeader = (struct Edp_DmxData_ChunkHeader*)(outData + sizeof(Edp_Commands));
    struct Edp_DmxData_PacketHeader* packetHeader = (struct Edp_DmxData_PacketHeader*)(outData + sizeof(Edp_Commands) + sizeof(Edp_DmxData_ChunkHeader));

    if (inDataSize != 0) {
        // Start a new packet, discard existing data and chunks

        memset(outData, 0x00, 600);

        // Loop over the input data so we know:
        //   - if it's all empty / zero
        //   - what the first and last used channels are so can send a sparse frame
        firstUsedChannel = 600;  // Some invalid value so we can detect if NO channel is in use
        lastUsedChannel = 600;   // Some invalid value so we can detect if NO channel is in use
        for (uint16_t i = 0; i < inDataSize; i++) {
            if ((firstUsedChannel == 600) && (inData[i] != 0)) {
                firstUsedChannel = i;
            }
            if (inData[i] != 0) {
                lastUsedChannel = i;
            }
        }

        // Special case: allZero packet
        if (lastUsedChannel == 600) {
            outData[0] = Edp_Commands::DmxDataAllZero;
            outData[1] = universeId;
            *thisChunkSize = 2;
            *callAgain = false;
            return true;
        }

        outData[0] = Edp_Commands::DmxData;
        packetHeader->universeId = universeId;

        limitedInDataSize = MIN(inDataSize, 512);

        prepareDmxData_chunkOffset = maxSendChunkSize;

        // TODO: Not yet supported
        // IF NOT SUPPORT SPARSE
        packetHeader->sparse = 0;
        packetHeader->sparseOffset = 0;
        sparseSize = 512;
        // ELSE
        packetHeader->sparse = 1;
        packetHeader->sparseOffset = MIN(firstUsedChannel, 255);
        sparseSize = MIN(lastUsedChannel, 511) - packetHeader->sparseOffset + 1;
        LOG("prepareDMX: firstUsedChannel: %u, lastUsedChannel: %u, sparseOffset: %u, sparseSize: %u", firstUsedChannel, lastUsedChannel, packetHeader->sparseOffset, sparseSize);

        // Compress inData to outData. If it's larger than the input, it will be overwritten later
        prepareDmxData_sizeOfDataToBeSent = 600 - sizeof(Edp_Commands) - sizeof(Edp_DmxData_ChunkHeader) - sizeof(Edp_DmxData_PacketHeader);
        destination = outData + sizeof(Edp_Commands) + sizeof(struct Edp_DmxData_ChunkHeader) + sizeof(Edp_DmxData_PacketHeader);
        snappy::RawCompress((const char *)inData + packetHeader->sparseOffset, sparseSize, (char*)destination, &prepareDmxData_sizeOfDataToBeSent);

        if (prepareDmxData_sizeOfDataToBeSent >= sparseSize) {
            LOG("Compressed size: %d (inSize: %d) => SENDING UNCOMPRESSED!", prepareDmxData_sizeOfDataToBeSent, sparseSize);
            packetHeader->compressed = 0;
            memcpy(destination, inData + packetHeader->sparseOffset, sparseSize);
            prepareDmxData_sizeOfDataToBeSent = sparseSize;
        } else {
            packetHeader->compressed = 1;
        }

        // Calculate a CRC so the receivers know if they got all the correct chunks
        // CRC is over the complete "payload" = without the PacketHeader
        packetHeader->crc = crc_init();
        packetHeader->crc = crc_update(packetHeader->crc, outData + sizeof(Edp_Commands) + sizeof(struct Edp_DmxData_ChunkHeader) + sizeof(Edp_DmxData_PacketHeader), prepareDmxData_sizeOfDataToBeSent);
        packetHeader->crc = crc_finalize(packetHeader->crc);

        // Increase the size of the packet by the prepended header
        prepareDmxData_sizeOfDataToBeSent += sizeof(struct Edp_DmxData_PacketHeader);

        LOG("Size with packetHeader: %u", prepareDmxData_sizeOfDataToBeSent);

        // Make chunk 0 ready
        chunkHeader->chunkCounter = Edp_DmxData_ChunkCounter::FirstPacket;
        if ((prepareDmxData_sizeOfDataToBeSent + sizeof(Edp_Commands) + sizeof (struct Edp_DmxData_ChunkHeader)) <= maxSendChunkSize) {
            // Yay, only one chunk needed :D
            chunkHeader->lastChunk = true;
            *thisChunkSize = prepareDmxData_sizeOfDataToBeSent + sizeof(Edp_Commands) + sizeof (struct Edp_DmxData_ChunkHeader);
            *callAgain = false;
            LOG("Only one chunk is needed :D Size: %u", prepareDmxData_sizeOfDataToBeSent + sizeof(Edp_Commands) + sizeof (struct Edp_DmxData_ChunkHeader));
            return true;
        }

        chunkHeader->lastChunk = false;

        *thisChunkSize = maxSendChunkSize;
        *callAgain = true;

        LOG("Chunk 0 is ready! :D Size: %u", maxSendChunkSize);

        return true;

    } else {
        // Next chunk if available, otherwise return false
        // TODO: Check if there actually is a next chunk or if this was accidentally
        //       called without inDataSize

        // ChunkOffset points to the OLD chunk's data

        destination = outData + sizeof(Edp_Commands) + sizeof(struct Edp_DmxData_ChunkHeader);
        memcpy(destination, outData + prepareDmxData_chunkOffset, maxSendChunkSize - sizeof(Edp_Commands) - sizeof(struct Edp_DmxData_ChunkHeader));

        chunkHeader->chunkCounter = (Edp_DmxData_ChunkCounter)(chunkHeader->chunkCounter + 1);

        LOG("Chunk %u is ready! chunkOffset: %u, maxSendChunkSize: %u, prepareDmxData_sizeOfDataToBeSent: %u",
            chunkHeader->chunkCounter,
            prepareDmxData_chunkOffset,
            maxSendChunkSize,
            prepareDmxData_sizeOfDataToBeSent);

        if (prepareDmxData_chunkOffset + maxSendChunkSize >= prepareDmxData_sizeOfDataToBeSent + sizeof(struct Edp_DmxData_PacketHeader)) {
            chunkHeader->lastChunk = true;
            *thisChunkSize = prepareDmxData_sizeOfDataToBeSent - prepareDmxData_chunkOffset + sizeof(struct Edp_DmxData_PacketHeader);
            *callAgain = false;
            LOG("It's the last chunk! Size: %u %04x", *thisChunkSize, *thisChunkSize);
            return true;
        }

        prepareDmxData_chunkOffset = prepareDmxData_chunkOffset + (maxSendChunkSize - sizeof(Edp_Commands) - sizeof(Edp_DmxData_ChunkHeader));
        *callAgain = true;

        return true;
    }
}

// Since every data source calling this has its own instance of EDP, this
// should be safe
bool Edp::processIncomingChunk(uint16_t chunkSize) {
    Patching patching;
    uint8_t universeId;
    uint16_t copySize;
    uint16_t crc;
    size_t uncompressedLength;

    if (chunkSize < 1) {
        return false;
    }

    patching.active = false;

    LOG("EDP INCOMING: %d byte. Command: %d", chunkSize, inData[0]);

    if (inData[0] == Edp_Commands::DmxDataAllZero) {
        // No chunk header, no packetheader, just the universeId
        patching = findPatching(inData[1]);

        LOG("allZero packet. universe: %u patching active: %u buffer: %u", inData[1], patching.active, patching.dstInstance);

        if (patching.active) {
            // Easy: Just clear the DmxBuffer
            dmxBuffer.zero(patching.dstInstance);
            return true;
        }
        return false;
    }

    if (inData[0] == Edp_Commands::DmxData) {
        // At least a chunk header + 1 byte payload needs to be there

        if (chunkSize < (sizeof(Edp_Commands) + sizeof(struct Edp_DmxData_ChunkHeader) + 1)) {
            return false;
        }

        struct Edp_DmxData_ChunkHeader* chunkHeader = (struct Edp_DmxData_ChunkHeader*)inData + sizeof(Edp_Commands);

        LOG("DmxData: Chunk: %d, LastChunk: %d", chunkHeader->chunkCounter, chunkHeader->lastChunk);

        // Complete frame (all chunks) is assembled in outData

        if (chunkHeader->chunkCounter == Edp_DmxData_ChunkCounter::FirstPacket) {
            // Clear outData so the following chunks comes in clean
            copySize = MIN((chunkSize - sizeof(Edp_Commands) - sizeof(struct Edp_DmxData_ChunkHeader)), 600);
            LOG("DmxData: FIRST chunk. Will copy %u byte", copySize);
            critical_section_enter_blocking(&bufferLock);
            memset(outData, 0x00, 600);
            memcpy(outData, inData + sizeof(Edp_Commands) + sizeof(struct Edp_DmxData_ChunkHeader), copySize);
            critical_section_exit(&bufferLock);
            prepareDmxData_chunkOffset = copySize;
        } else if (chunkHeader->chunkCounter < 32) {
            // Some intermediate packet: Just copy it to outData
            copySize = MIN((chunkSize - sizeof(Edp_Commands) - sizeof(struct Edp_DmxData_ChunkHeader)), 600);
            LOG("DmxData: INTERMEDIATE chunk. Will copy %u at offset %u", copySize, prepareDmxData_chunkOffset);
            critical_section_enter_blocking(&bufferLock);
            memcpy(outData + prepareDmxData_chunkOffset, inData + sizeof(Edp_Commands) + sizeof(struct Edp_DmxData_ChunkHeader), copySize);
            critical_section_exit(&bufferLock);
            prepareDmxData_chunkOffset += copySize;
        }

        // If the last chunk just came in, we have everything and can act on it
        if (chunkHeader->lastChunk) {
            struct Edp_DmxData_PacketHeader* packetHeader = (struct Edp_DmxData_PacketHeader*)outData;

            // Check CRC and discard packet if it doesn't match
            LOG("Checksum first byte: %02x, len: %u", (outData + sizeof(struct Edp_DmxData_PacketHeader))[0], prepareDmxData_chunkOffset - sizeof(struct Edp_DmxData_PacketHeader));
            crc = crc_init();
            crc = crc_update(crc, outData + sizeof(struct Edp_DmxData_PacketHeader), prepareDmxData_chunkOffset - sizeof(struct Edp_DmxData_PacketHeader));
            crc = crc_finalize(crc);
            if (crc != packetHeader->crc) {
                LOG("CRC mismatch! Expected: %04x, Calculated: %04x", packetHeader->crc, crc);
                return false;
            }

            // The complete packet (compressed or not) sits at outData
            // inData contains the last chunk received + possibly garbage

            // For sparse packets to work, we need 512 byte of zeroed space, so we
            // will reuse inData for that. So zero it here
            memset(inData, 0x00, 600);

            patching = findPatching(packetHeader->universeId);

            LOG("DmxData packet complete! universe: %u, packetLen: %u, compressed: %u, sparse: %u, sparseOffset: %u, patching active: %u buffer: %u",
                packetHeader->universeId,
                prepareDmxData_chunkOffset,
                packetHeader->compressed,
                packetHeader->sparse,
                packetHeader->sparseOffset,
                patching.active,
                patching.dstInstance
            );

            // If this universe is not patched, no need to do anything
            if (!patching.active) {
                return true; // TODO: or better false?
            }

            if (packetHeader->compressed) {
                if (snappy::GetUncompressedLength((const char*)(outData + sizeof(struct Edp_DmxData_PacketHeader)), prepareDmxData_chunkOffset - sizeof(struct Edp_DmxData_PacketHeader), &uncompressedLength) == true) {
                    LOG("snappy::GetUncompressedLength: %d", uncompressedLength);

                    // Sanity check: uncompressedLength must be 512 OR the frame is sparse
                    if ((!packetHeader->sparse && uncompressedLength != 512) || (packetHeader->sparse && uncompressedLength > 512)) {
                        return false;
                    }

                    if (snappy::RawUncompress((const char*)(outData + sizeof(struct Edp_DmxData_PacketHeader)), prepareDmxData_chunkOffset - sizeof(struct Edp_DmxData_PacketHeader), (char*)inData + packetHeader->sparseOffset) == true) {
                        dmxBuffer.setBuffer(patching.dstInstance, inData, uncompressedLength + packetHeader->sparseOffset);
                        return true;
                    } else {
                        LOG("snappy::RawUncompress failed :(");
                        return false;
                    }
                } else {
                    LOG("snappy::GetUncompressedLength failed :(");
                    return false;
                }
            } else {
                // Sanity check: if full frame, packetLen MUST be 512 + sizeof PacketHeader
                if (prepareDmxData_chunkOffset == (512 + sizeof(Edp_DmxData_PacketHeader))) {
                    dmxBuffer.setBuffer(patching.dstInstance, outData + sizeof(struct Edp_DmxData_PacketHeader), (prepareDmxData_chunkOffset - sizeof(struct Edp_DmxData_PacketHeader)));
                    return true;
                } else if (packetHeader->sparse) {
                    memcpy(inData + packetHeader->sparseOffset, outData + sizeof(struct Edp_DmxData_PacketHeader), prepareDmxData_chunkOffset - sizeof(struct Edp_DmxData_PacketHeader));
                    dmxBuffer.setBuffer(patching.dstInstance, inData, prepareDmxData_chunkOffset + packetHeader->sparseOffset);
                    return true;
                }
                return false;
            }
        }
    }

    // Should not reach here!
    return false;
}

// Find a patching patching from ETH -> buffer. All other patching destination
// are NOT supported for now
Patching Edp::findPatching(uint8_t universeId) {
    // Fallback in case we don't find a match
    Patching retPatch;
    retPatch.active = false;

    for (int i = 0; i < MAX_PATCHINGS; i++) {
        Patching patching = boardConfig.activeConfig->patching[i];
        if ((!patching.active) ||
            (patching.srcType != patchSource) ||
            (patching.srcInstance != universeId))
        {
            continue;
        }
        return patching;
    }

    return retPatch;
}
