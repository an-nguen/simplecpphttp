//
// Created by an on 10/18/20.
//

#ifndef CPPHTTP_SIMPLELOGGER_H
#define CPPHTTP_SIMPLELOGGER_H

#include "AbstractLogger.h"

class SimpleLogger : public logs::AbstractLogger {
public:
    void info(std::basic_string<char> msg) override;

    void trace(std::basic_string<char> msg) override;

    void debug(std::basic_string<char> msg) override;

    void warn(std::basic_string<char> msg) override;

    void error(std::basic_string<char> msg) override;

    void fatal(std::basic_string<char> msg) override;
};


#endif //CPPHTTP_SIMPLELOGGER_H
