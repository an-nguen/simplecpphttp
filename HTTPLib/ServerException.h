//
// Created by an on 10/20/20.
//

#ifndef CPPHTTP_SERVEREXCEPTION_H
#define CPPHTTP_SERVEREXCEPTION_H

#include <exception>
#include <string>
#include <utility>

namespace cpphttp {
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define SERVER_EXCEPTION(msg) ServerException(std::string(__FILE__) + ":" + TOSTRING(__LINE__) + " - " + msg)

    class ServerException: public std::exception {
    public:
        explicit ServerException(std::basic_string<char> message) {
            m_message = "\n\033[1;31mServerException:\n\t";
            m_message.append(message);
            m_message.append("\n\033[0m");
        }

        const char *what() const _GLIBCXX_TXN_SAFE_DYN _GLIBCXX_NOTHROW override {
            return m_message.c_str();
        }

    private:
        std::string m_message{};
    };
}

#endif //CPPHTTP_SERVEREXCEPTION_H
