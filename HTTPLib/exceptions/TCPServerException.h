//
// Created by an on 10/20/20.
//

#ifndef CPPHTTP_TCPSERVEREXCEPTION_H
#define CPPHTTP_TCPSERVEREXCEPTION_H

#include <exception>
#include <string>
#include <utility>

#include "../macro/HTTP_MACRO.h"

namespace cpphttp {
    class TCPServerException: public std::exception {
    public:
        explicit TCPServerException(const std::basic_string<char>& message) {
            m_message = fmt::format("\n\033[1;31mTCPServerException:\n\t {}:{} - {} \n\033[0m", __FILE__, __LINE__, message);
        }

        template<SameString ...T>
        explicit TCPServerException(T... args) {
            m_message = fmt::format("\n\033[1;31mTCPServerException:\n\t {}:{} - ", __FILE__, __LINE__);

            std::vector<std::basic_string<char>> values = {args...};
            for (auto &arg: values)
                m_message.append(arg);
            m_message.append("\n\033[0m");
        }

        [[nodiscard]] const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
            return m_message.c_str();
        }

    private:
        std::string m_message{};
    };
}

#endif //CPPHTTP_TCPSERVEREXCEPTION_H
