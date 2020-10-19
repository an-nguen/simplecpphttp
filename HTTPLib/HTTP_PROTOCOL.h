//
// Created by an on 12.03.2020.
//

#ifndef CPPHTTP_HTTP_PROTOCOL_H
#define CPPHTTP_HTTP_PROTOCOL_H

#include <cstring>
#include "HTTP_MACRO.h"

namespace cpphttp {
#define HTTP_PROTOCOLS(FUNC) \
                FUNC(NO_PROTOCOL)\
                FUNC(HTTP_1_0)\
                FUNC(HTTP_1_1)\
                FUNC(HTTP_2)

    enum HTTP_PROTOCOL_ENUM {
        HTTP_PROTOCOLS(GEN_ENUM)
    };

    static const char * HTTP_PROTOCOL_STR[] = {
            "NO_PROTOCOL",
            "HTTP/1.0",
            "HTTP/1.1",
            "HTTP/2",
    };

    static HTTP_PROTOCOL_ENUM getHTTPProtocol(const char * str) {
        for (unsigned i = 0; i < (sizeof(HTTP_PROTOCOL_STR) / sizeof(HTTP_PROTOCOL_STR[0])); i++)
            if (strstr(str, HTTP_PROTOCOL_STR[i]) != nullptr)
                return (HTTP_PROTOCOL_ENUM)i;

        throw std::runtime_error("no http protocol");
    }
}

#endif //CPPHTTP_HTTP_PROTOCOL_H
