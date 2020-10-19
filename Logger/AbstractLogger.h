//
// Created by an on 10/18/20.
//

#ifndef CPPHTTP_ABSTRACTLOGGER_H
#define CPPHTTP_ABSTRACTLOGGER_H

namespace logs {

    class AbstractLogger {
    public:
        virtual void info(const std::basic_string<char> msg) = 0;

        virtual void trace(const std::basic_string<char> msg) = 0;

        virtual void debug(const std::basic_string<char> msg) = 0;

        virtual void warn(const std::basic_string<char> msg) = 0;

        virtual void error(const std::basic_string<char> msg) = 0;

        virtual void fatal(const std::basic_string<char> msg) = 0;
    };

    template <class T>
    concept DerivedAbstractLogger = std::is_base_of<logs::AbstractLogger, T>::value;

}


#endif //CPPHTTP_ABSTRACTLOGGER_H
