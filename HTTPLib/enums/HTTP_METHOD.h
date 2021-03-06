#ifndef HTTPMETHOD_H
#define HTTPMETHOD_H

#include <map>
#include <cstring>
#include "../macro/HTTP_MACRO.h"

namespace cpphttp {
#define HTTP_METHODS(FUNC) \
                FUNC(GET)\
                FUNC(HEAD)\
                FUNC(POST)\
                FUNC(PUT)\
                FUNC(DELETE)\
                FUNC(CONNECT)\
                FUNC(OPTIONS)\
                FUNC(TRACE)\
                FUNC(PATCH)\
                FUNC(NO_METHOD)


    enum HTTP_METHOD {
        HTTP_METHODS(GEN_ENUM)
    };

    static std::array HTTP_METHOD_STR = {
        HTTP_METHODS(GEN_STR)
    };

    static auto getHTTPMethod(std::basic_string<char>& str) {
        for (unsigned long i = 0; i < HTTP_METHOD_STR.size(); i++)
            if (str == HTTP_METHOD_STR.at(i))
                return HTTP_METHOD (i);

        throw std::runtime_error("http method not found");
    }

    static const char *getHTTPMethodStr(HTTP_METHOD method) {
        return HTTP_METHOD_STR[method];
    }
}

#endif // HTTPMETHOD_H
