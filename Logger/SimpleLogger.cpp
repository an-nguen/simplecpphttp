//
// Created by an on 10/18/20.
//

#include <iostream>
#include <iomanip>
#include <chrono>

#include "SimpleLogger.h"

// output template = 'date time    level  :  message'

#define PRINT_LOG(level, message) auto nowTimePoint = std::chrono::system_clock::now(); \
auto time = std::chrono::system_clock::to_time_t(nowTimePoint);                               \
std::cout << std::put_time(std::localtime(&time), "%Y-%m-%d %X") << std::string(4, ' ') << "[" << level << "]:  " \
<< msg << std::endl

void SimpleLogger::info(const std::basic_string<char> msg) {
    PRINT_LOG("\033[1;37mINFO\033[0m", msg);
}

void SimpleLogger::trace(const std::basic_string<char> msg) {
    PRINT_LOG("\033[1;36mTRACE\033[0m", msg);
}

void SimpleLogger::debug(const std::basic_string<char> msg) {
    PRINT_LOG("\033[1;32mDEBUG\033[0m", msg);
}

void SimpleLogger::warn(const std::basic_string<char> msg) {
    PRINT_LOG("\033[1;33mWARN\033[0m", msg);
}

void SimpleLogger::error(const std::basic_string<char> msg) {
    PRINT_LOG("\033[1;31mERROR\033[0m", msg);
}

void SimpleLogger::fatal(const std::basic_string<char> msg) {
    PRINT_LOG("\033[1;31mFATAL\033[0m", msg);
    exit(1);
}
