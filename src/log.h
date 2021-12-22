#ifndef LOG_H
#define LOG_H

#include "pico/stdlib.h"
#include "pico/mutex.h"
#include "pico/util/queue.h"

#ifdef __cplusplus

#include <string>

#define LOG_BUFFER_SIZE     100

// TODO: LOG MASKS
#define LOG_MASK_ARTNET      0x00000001
#define LOG_MASK_WIRELESS    0x00000002
#define LOG_MASK_DMXBUFFER   0x00000004

class Log {
  public:
    void init();
    static void dlog(char* file, uint32_t line, char* text);
    static size_t getLogBufferNumEntries();
    static size_t getLogBuffer(char* buffer, size_t size);
    static void clearLogBuffer();

  private:
    static uint32_t logLineCount;
    static queue_t logQueue;
    static mutex_t logLock;
    static char logLine[255];
};

#endif // __cplusplus

// Helper methods which are called from C code
#ifdef __cplusplus
extern "C" {
#endif

// TODO: Instead of heaving just a (nice) macro, wrap printf (which is already
//       wrapped by the pico-sdk). If possible ...
//       https://www.raspberrypi.org/forums/viewtopic.php?f=145&t=315365

#define LOG(text, ...) dlog((char*)__FILE__, __LINE__, (char*)text, ##__VA_ARGS__)

void dlog(char* file, uint32_t line, char* text, ...);

#ifdef __cplusplus
}
#endif

#endif // LOG_H
