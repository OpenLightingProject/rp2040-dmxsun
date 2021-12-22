#include "log.h"

#include <stdarg.h>
#include <stdio.h>

#include <tusb.h>

uint32_t Log::logLineCount;
queue_t Log::logQueue;
mutex_t Log::logLock;
char Log::logLine[255];

void Log::init() {
    mutex_init(&logLock);
    queue_init(&logQueue, 255, LOG_BUFFER_SIZE);
    Log::logLineCount = 0;
}

void Log::dlog(char* file, uint32_t line, char* text) {
    // Input sanitation so that one cannot make it invalid JSON
    // TODO: RegEx is damn slow and memory-hungry. Any idea for something leaner?
    //std::string bufSanitized = std::string(text);
    //bufSanitized = std::regex_replace(bufSanitized, std::regex("\""), "\\\"");
    //bufSanitized = std::regex_replace(bufSanitized, std::regex("\n"), "\\n");

    std::string fname = std::string(file);
    auto const pos = fname.find_last_of('/');
    fname = fname.substr(pos + 1);

    // If ACM console IS connected, just print it
    // If ACM console is not connected, append to log buffer (of course size-limitig it)
    if (tud_cdc_connected()) {
        printf("{\"type\": \"log\", \"count\": %ld, \"core\": %u, \"file\": \"%s\", \"line\": %ld, \"text\": \"%s\"}\n", logLineCount, get_core_num(), fname.c_str(), line, text);
    } else {
        mutex_enter_blocking(&logLock);
        if (queue_is_full(&logQueue)) {
            queue_remove_blocking(&logQueue, logLine);
        }
        snprintf(logLine, 255, "{\"type\": \"log\", \"count\": %ld, \"core\": %u, \"file\": \"%s\", \"line\": %ld, \"text\": \"%s\"}\n", logLineCount, get_core_num(), fname.c_str(), line, text);
        queue_add_blocking(&logQueue, logLine);
        mutex_exit(&logLock);
    }

    Log::logLineCount++;
}

size_t Log::getLogBufferNumEntries() {
    return queue_get_level(&logQueue);
}

size_t Log::getLogBuffer(char* buffer, size_t size) {
    size_t offset = 0;

    if (buffer == 0) {
        return 0;
    }

    // Avoid an underflow later when the loop is not run but we
    // subtract from the offset anyways
    if (size < 12) {
        return offset;
    }

    while(!queue_is_empty(&logQueue))
    {
        // Make sure we have enough space left in the buffer
        if ((size - offset) < 257) {
            break;
        }
        queue_remove_blocking(&logQueue, logLine);
        offset += snprintf(buffer + offset, size - offset, "%s,\n", logLine);
    }

    // Remove the last comma and line break
    if (offset > 5) {
        offset -= 2;
    }
    mutex_exit(&logLock);

    return offset;
}

void Log::clearLogBuffer() {
    mutex_enter_blocking(&logLock);
    while (!queue_is_empty(&logQueue))
    {
        queue_remove_blocking(&logQueue, logLine);
    }
    mutex_exit(&logLock);
}

// C helper functions
void dlog(char* file, uint32_t line, char* text, ...) {
    va_list args;
    char buf[1024];

    va_start(args, text);
    vsnprintf(buf, 1024, text, args);
    va_end(args);

    Log::dlog(file, line, buf);
}