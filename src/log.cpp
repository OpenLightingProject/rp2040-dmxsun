#include "log.h"

#include <sstream>
#include <iterator>
#include <regex>

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include <tusb.h>

std::vector<std::string> Log::logBuffer;
uint32_t Log::logLineCount;
mutex_t Log::logLock;

void Log::init() {
    Log::logBuffer.clear();
    Log::logLineCount = 0;
    mutex_init(&logLock);
}

void Log::dlog(char* file, uint32_t line, char* text) {
    // Input sanitation so that one cannot make it invalid JSON
    std::string bufSanitized = std::string(text);
    //bufSanitized = std::regex_replace(bufSanitized, std::regex("\""), "\\\"");
    //bufSanitized = std::regex_replace(bufSanitized, std::regex("\n"), "\\n");

    std::string fname = std::string(file);
    auto const pos = fname.find_last_of('/');
    fname = fname.substr(pos + 1);

    // If ACM console is not connected, append to log buffer (of course size-limitig it)
    // If ACM console IS connected, just print it
    if (tud_cdc_connected()) {
        printf("{\"type\": \"log\", \"count\": %ld, \"file\": \"%s\", \"line\": %ld, \"text\": \"%s\"}\n", logLineCount, fname.c_str(), line, bufSanitized.c_str());
    } else {
        // TODO: A mutex makes sure that only one core is inside here.
        //       However, it might block the core from more important tasks!
        // TODO: Maybe use "faster" data structures?
        // TODO: Disable logging on "production" builds?
        mutex_enter_blocking(&logLock);
        if (Log::logBuffer.size() >= 30) {
            Log::logBuffer.erase(Log::logBuffer.begin());
        }
        Log::logBuffer.push_back(
            std::string(
                "{" +
                std::string("\"type\": \"log\", ") +
                std::string("\"count\": " + std::to_string(logLineCount) + ", ") +
                std::string("\"file\": \"" + fname + "\", ") +
                std::string("\"line\": " + std::to_string(line) + ", ") +
                std::string("\"text\": \"" + bufSanitized + "\"") +
                "}"
            )
        );
        mutex_exit(&logLock);
    }

    Log::logLineCount++;
}

size_t Log::getLogBufferNumEntries() {
    size_t size = 0;

    mutex_enter_blocking(&logLock);
    size = Log::logBuffer.size();
    mutex_exit(&logLock);

    return size;
}

std::string Log::getLogBuffer(int maxSize) {
    std::string output;
    uint16_t elements = 0;

    // Avoid an underflow later
    if (maxSize < 6) {
        return output;
    }

    mutex_enter_blocking(&logLock);
    for (const auto& value: Log::logBuffer) {
        if (output.size() < (maxSize - 5)) {
            output += value + ",\n";
            elements++;
        }
    }

    // Drop the elements we have in output from the logBuffer
    Log::logBuffer.erase(Log::logBuffer.begin(), Log::logBuffer.begin() + elements);
    mutex_exit(&logLock);

    // Remove the last comma and line break
    if (output.size() > 5) {
        output.pop_back();
        output.pop_back();
    }

    return output;
}

void Log::clearLogBuffer() {
    mutex_enter_blocking(&logLock);
    Log::logBuffer.clear();
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