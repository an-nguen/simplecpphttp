//
// Created by an on 10/20/20.
//

#ifndef CPPHTTP_TCPCLIENTEXCEPTION_H
#define CPPHTTP_TCPCLIENTEXCEPTION_H

#include <fmt/core.h>
#include "../macro/HTTP_MACRO.h"
#include "../concepts/http_concepts.h"

namespace cpphttp {
    class TCPClientException: public std::exception {
    public:
        explicit TCPClientException(const std::basic_string<char>& message) {
            m_message = fmt::format("\n\033[1;31mTCPClientException:\n\t {}:{} - {} \n\033[0m", __FILE__, __LINE__, message);
        }

        template<SameString ...T>
        explicit TCPClientException(T... args) {
            m_message = fmt::format("\n\033[1;31mTCPClientException:\n\t {}:{} - ", __FILE__, __LINE__);

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

#endif //CPPHTTP_TCPCLIENTEXCEPTION_H
