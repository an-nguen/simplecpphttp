#ifndef HTTPMETHOD_H
#define HTTPMETHOD_H

#include <map>
#include <cstring>
#include "HTTP_MACRO.h"

namespace CPPHTTP {
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

    static const char * HTTP_METHOD_STR[] = {
        HTTP_METHODS(GEN_STR)
    };

    static HTTP_METHOD getHTTPMethod(const char * str) {
        for (int i = 0; i < (sizeof(HTTP_METHOD_STR) / sizeof(HTTP_METHOD_STR[0])); i++)
            if (strstr(str, HTTP_METHOD_STR[i]) != nullptr)
                return (HTTP_METHOD)i;

        return (HTTP_METHOD)-1;
    }

    static const char *getHTTPMethodStr(HTTP_METHOD method) {
        return HTTP_METHOD_STR[method];
    }
}

#endif // HTTPMETHOD_H