#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

struct LogData {
    const char* file;
    int line;
    const char* func;
    LogData(const char* fi, int li, const char* fu){
        file = fi;
        line = li;
        func = fu;
    }
};
#define __LOG_DATA__ LogData(__FILE_NAME__,__LINE__,__func__)

class Logger {
private:
    enum Code {
        // Foreground
        FG_BLACK           = 30,
        FG_RED             = 31,
        FG_GREEN           = 32,
        FG_YELLOW          = 33,
        FG_BLUE            = 34,
        FG_MAGENTA         = 35,
        FG_CYAN            = 36,
        FG_WHITE           = 37,
        FG_DEFAULT         = 39,

        FG_BRIGHT_BLACK    = 90,
        FG_BRIGHT_RED      = 91,
        FG_BRIGHT_GREEN    = 92,
        FG_BRIGHT_YELLOW   = 93,
        FG_BRIGHT_BLUE     = 94,
        FG_BRIGHT_MAGENTA  = 95,
        FG_BRIGHT_CYAN     = 96,
        FG_BRIGHT_WHITE    = 97,

        // Background
        BG_BLACK           = 40,
        BG_RED             = 41,
        BG_GREEN           = 42,
        BG_YELLOW          = 43,
        BG_BLUE            = 44,
        BG_MAGENTA         = 45,
        BG_CYAN            = 46,
        BG_WHITE           = 47,
        BG_DEFAULT         = 49,

        BG_BRIGHT_BLACK    = 100,
        BG_BRIGHT_RED      = 101,
        BG_BRIGHT_GREEN    = 102,
        BG_BRIGHT_YELLOW   = 103,
        BG_BRIGHT_BLUE     = 104,
        BG_BRIGHT_MAGENTA  = 105,
        BG_BRIGHT_CYAN     = 106,
        BG_BRIGHT_WHITE    = 107,
    };
    static std::string color(Code code);
    static void logData(const LogData& data);
public:
    static void logInBrackets(const std::string& msg);
    static void log(const std::string& msg, const LogData& data);
    static void logInfo(const std::string& msg, const LogData& data);
    static void logWarning(const std::string& msg, const LogData& data);
    static void logError(const std::string& msg, const LogData& data);
    static void logSuccess(const std::string& msg, const LogData& data);
    static void logFailure(const std::string& msg, const LogData& data);
};

#endif