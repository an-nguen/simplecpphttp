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
#define SERVER_EXCEPTION2(...) ServerException(std::string(__FILE__) + ":" + TOSTRING(__LINE__) + " - ", ##__VA_ARGS__)

    template <class T>
    concept SameString = std::is_same<T, std::basic_string<char>>::value ||
    std::is_same<T, const char *>::value || std::is_same<T, char *>::value;

    class ServerException: public std::exception {
    public:
        explicit ServerException(const std::basic_string<char>& message) {
            m_message = "\n\033[1;31mServerException:\n\t";
            m_message.append(message);
            m_message.append("\n\033[0m");
        }

        template<SameString ...T>
        explicit ServerException(T... args) {
            m_message = "\n\033[1;31mServerException:\n\t";
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

#endif //CPPHTTP_SERVEREXCEPTION_H
