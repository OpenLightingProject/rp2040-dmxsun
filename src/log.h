#ifndef LOG_H
#define LOG_H

#include "pico/stdlib.h"

#ifdef __cplusplus
extern "C" {
#endif

// TODO: Instead of heaving just a (nice) macro, wrap printf (which is already wrapped by the pico-sdk)

#define LOG(text, ...) dlog((char*)__FILE__, __LINE__, (char*)text, ##__VA_ARGS__)

void dlog(char* file, uint32_t line, char* text, ...);
uint32_t getLogBuffer(char* buf, uint32_t sizeOfBuf);
void clearLugBuffer();

#ifdef __cplusplus
}
#endif

#endif // LOG_H
