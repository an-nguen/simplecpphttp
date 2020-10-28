//
// Created by an on 12.03.2020.
//

#ifndef CPPHTTP_HTTP_MACRO_H
#define CPPHTTP_HTTP_MACRO_H

namespace cpphttp {
#define GEN_ENUM(val) val ,
#define GEN_STR(val) #val ,
#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

    constexpr auto HTTP_HEADER_BODY_DIVIDER = "\r\n\r\n";
    constexpr auto HTTP_NEW_LINE_CHARACTERS = "\r\n";
}
#endif //CPPHTTP_HTTP_MACRO_H
