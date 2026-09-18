#include "logger.hpp"

#include <iostream>

using namespace std;

string Logger::color(Code code){
    return "\033[" + to_string((int)code) + "m";
}

void Logger::logData(const LogData& data){
    logInBrackets(string(data.file) + ":" + to_string(data.line) + ":" + string(data.func));
}

void Logger::logInBrackets(const std::string& msg){
    cout << color(Code::FG_BLACK) << "[" << color(Code::FG_DEFAULT);
    cout << msg;
    cout << color(Code::FG_BLACK) << "] " << color(Code::FG_DEFAULT);
}

void Logger::log(const std::string& msg, const LogData& data){
    logData(data);
    cout << msg << endl;
}

void Logger::logInfo(const std::string& msg, const LogData& data){
    logInBrackets("INFO");
    logData(data);
    cout << color(Code::FG_BRIGHT_BLUE) << msg << color(Code::FG_DEFAULT) << endl;
}

void Logger::logWarning(const std::string& msg, const LogData& data){
    logInBrackets("WARN");
    logData(data);
    cout << color(Code::FG_BRIGHT_YELLOW) << msg << color(Code::FG_DEFAULT) << endl;
}

void Logger::logError(const std::string& msg, const LogData& data){
    logInBrackets("ERR");
    logData(data);
    cout << color(Code::FG_BRIGHT_RED) << msg << color(Code::FG_DEFAULT) << endl;
}

void Logger::logSuccess(const std::string& msg, const LogData& data){
    logInBrackets("SUCCESS");
    logData(data);
    cout << color(Code::FG_BRIGHT_GREEN) << msg << color(Code::FG_DEFAULT) << endl;
}

void Logger::logFailure(const std::string& msg, const LogData& data){
    logInBrackets("FAIL");
    logData(data);
    cout << color(Code::FG_BRIGHT_RED) << msg << color(Code::FG_DEFAULT) << endl;
}
