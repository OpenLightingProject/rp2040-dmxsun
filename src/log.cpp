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

    std::string bufSanitized = std::string(buf);
    bufSanitized = std::regex_replace(bufSanitized, std::regex("\""), "\\\"");
    bufSanitized = std::regex_replace(bufSanitized, std::regex("\n"), "\\n");

    std::string fname = std::string(file);
    auto const pos = fname.find_last_of('/');
    fname = fname.substr(pos + 1);

    printf("{type: \"log\", file: \"%s\", line: %ld, text: \"%s\"}\n", fname.c_str(), line, bufSanitized.c_str());

/*
    logBuffer.push_back(
        std::string("{type: \"log\", file: \"" + fname +
        std::string("\", line: ") + std::to_string(line) +
        std::string(", text: \"") + std::string(text) + std::string("\"}")
    ));
*/
}

uint32_t getLogBuffer(char* buf, uint32_t sizeOfBuf) {
    const char* const delim = "\n";

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