//
// Created by an on 10/27/20.
//

#ifndef CPPHTTP_HTTPHANDLEREXCEPTION_H
#define CPPHTTP_HTTPHANDLEREXCEPTION_H

#include "TCPServerException.h"
#include <vector>

namespace cpphttp {
    class HTTPHandlerException: std::exception {
    public:
        explicit HTTPHandlerException(const std::basic_string<char>& message) {
            m_message = "\n\033[1;31mServerException:\n\t";
            m_message.append(message);
            m_message.append("\n\033[0m");
        }

        template<SameString ...T>
        explicit HTTPHandlerException(T... args) {
            m_message = "\n\033[1;31mServerException:\n\t";
            std::vector<std::basic_string<char>> values = {args...};
            for (auto &arg: values)
                m_message.append(arg);
            m_message.append("\n\033[0m");
        }
        [[nodiscard]] const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
            return exception::what();
        }

    private:
        std::string m_message{};
    };
}

#endif //CPPHTTP_HTTPHANDLEREXCEPTION_H
