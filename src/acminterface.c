#include "acminterface.h"


// Prints a complete universe in a human-friendly format
void debugPrintUniverse(uint8_t universe) {
    uint16_t offset;
    uint16_t chan;

    printf("%02d  01 02 03 04 05 06 07 08 09 10 11 12 13 14 15 16 17 18 19 20\n", universe);

    for (offset = 0; offset < 510; offset += 20) {
        printf("%03d ", offset);
        for (chan = offset; chan < (offset + 20); chan++) {
            if (chan < 512) {
                printf("%02x ", dmx_values[universe][chan]);
            }
        }
        printf("\n");
    }
    printf("\n");
}
