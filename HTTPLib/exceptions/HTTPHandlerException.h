//
// Created by an on 10/27/20.
//

#ifndef CPPHTTP_HTTPHANDLEREXCEPTION_H
#define CPPHTTP_HTTPHANDLEREXCEPTION_H

#include "../concepts/http_concepts.h"
#include <fmt/core.h>

namespace cpphttp {
    class HTTPHandlerException: std::exception {
    public:
        explicit HTTPHandlerException(const std::basic_string<char>& message) {
            m_message = fmt::format("\n\033[1;31mHTTPHandlerException:\n\t {}:{} - {} \n\033[0m", __FILE__, __LINE__, message);
        }

        template<SameString ...T>
        explicit HTTPHandlerException(T... args) {
            m_message = fmt::format("\n\033[1;31mHTTPHandlerException:\n\t {}:{} - ", __FILE__, __LINE__);

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

#endif //CPPHTTP_HTTPHANDLEREXCEPTION_H
