#ifndef BOARDCONFIG_H
#define BOARDCONFIG_H

#include "pins.h"

class BoardConfig {
  public:
    void init();
    void readIOBoards();

  private:
    bool responding[4];       // True if the board resonded to the bus scan
    uint8_t rawData[5][256];  // raw content of the memories (0-3: 4 IO boards, 4: baseboard, 256 byte each)
};

#endif // BOARDCONFIG_H
