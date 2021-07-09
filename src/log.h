#ifndef LOG_H
#define LOG_H

#include "pico/stdlib.h"

#ifdef __cplusplus

#include <string>
#include <vector>

class Log {
  public:
    void init();
    static void dlog(char* file, uint32_t line, char* text);
    static size_t getLogBufferNumEntries();
    static std::string getLogBuffer(int maxSize);
    static void clearLogBuffer();

  private:
    static std::vector<std::string> logBuffer;
    static uint32_t logLineCount;
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
