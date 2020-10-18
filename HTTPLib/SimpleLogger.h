//
// Created by an on 10/18/20.
//

#ifndef CPPHTTP_SIMPLELOGGER_H
#define CPPHTTP_SIMPLELOGGER_H

#include "AbstractLogger.h"

class SimpleLogger : public logs::AbstractLogger {
public:
    void info(const char *msg) override;

    void trace(const char *msg) override;

    void debug(const char *msg) override;

    void warn(const char *msg) override;

    void error(const char *msg) override;

    void fatal(const char *msg) override;
};


#endif //CPPHTTP_SIMPLELOGGER_H
