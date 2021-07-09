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

void Log::init() {
    Log::logBuffer.clear();
    Log::logLineCount = 0;
}

void Log::dlog(char* file, uint32_t line, char* text) {
    // Input sanitation so that one cannot make it invalid JSON
    std::string bufSanitized = std::string(text);
    bufSanitized = std::regex_replace(bufSanitized, std::regex("\""), "\\\"");
    bufSanitized = std::regex_replace(bufSanitized, std::regex("\n"), "\\n");

    std::string fname = std::string(file);
    auto const pos = fname.find_last_of('/');
    fname = fname.substr(pos + 1);

    // If ACM console is not connected, append to log buffer (of course size-limitig it)
    // If ACM console IS connected, just print it
    if (tud_cdc_connected()) {
        printf("{type: \"log\", count: %ld, file: \"%s\", line: %ld, text: \"%s\"}\n", logLineCount, fname.c_str(), line, bufSanitized.c_str());
    } else {
        if (Log::logBuffer.size() >= 30) {
            Log::logBuffer.erase(Log::logBuffer.begin());
        }
        Log::logBuffer.push_back(
            std::string(
                "{" +
                std::string("type: \"log\", ") +
                std::string("count: " + std::to_string(logLineCount) + ", ") +
                std::string("file: \"" + fname + "\", ") +
                std::string("line: " + std::to_string(line) + ", ") +
                std::string("text: \"" + bufSanitized + "\"") +
                "}"
            )
        );
    }

    Log::logLineCount++;
}

std::string Log::getLogBuffer() {
    std::string output;
    for (const auto& value: Log::logBuffer) {
        output += value + ",\n";
    }

    return output;
}

void Log::clearLogBuffer() {
    Log::logBuffer.clear();
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