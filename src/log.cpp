#include <vector>
#include <string>
#include <sstream>
#include <iterator>
#include <regex>

#include <string.h>

static std::vector<std::string> logBuffer;

extern "C" {
#include "log.h"

#include <stdarg.h>
#include <stdio.h>

void dlog(char* file, uint32_t line, char* text, ...) {
    va_list args;
    char buf[1024];

    va_start(args, text);
    vsnprintf(buf, 1024, text, args);
    va_end(args);

    // Input sanitation so that one cannot make it invalid JSON
    std::string bufSanitized = std::string(buf);
    bufSanitized = std::regex_replace(bufSanitized, std::regex("\""), "\\\"");
    bufSanitized = std::regex_replace(bufSanitized, std::regex("\n"), "\\n");

    std::string fname = std::string(file);
    auto const pos = fname.find_last_of('/');
    fname = fname.substr(pos + 1);

    // If ACM console is not connected, append to log buffer (of course size-limitig it)
    // If ACM console IS connected, just print it
    if (tud_cdc_connected()) {
        printf("{type: \"log\", file: \"%s\", line: %ld, text: \"%s\"}\n", fname.c_str(), line, bufSanitized.c_str());
    } else {
        if (logBuffer.size >= 30) {
            logBuffer.erase(logBuffer.begin());
        }
        logBuffer.push_back(
            std::string("{type: \"log\", file: \"" + fname +
            std::string("\", line: ") + std::to_string(line) +
            std::string(", text: \"") + bufSanitized + std::string("\"}")
        ));
    }
}

uint32_t getLogBuffer(char* buf, uint32_t sizeOfBuf) {
    std::string output;
    for (const auto& value: logBuffer) {
        output += value + "\n";
    }

    uint32_t size = output.length() + 1;
    if (sizeOfBuf < size) {
        size = sizeOfBuf - 2;
    }

    if (buf) {
        memset(buf, 0, sizeOfBuf);
        memcpy(buf, output.c_str(), size);
    }

    return size;
}

void clearLugBuffer() {
    logBuffer.clear();
}

}