//
// Created by an on 09.02.2020.
//

#ifndef CPPHTTP_HTTPUTILS_H
#define CPPHTTP_HTTPUTILS_H

#include <vector>
#include <string>

namespace CPPHTTP {
    class HTTPUtils {
    public:
        static std::vector <std::string> split (const std::string& str, const std::string& divider);
    };
}


#endif //CPPHTTP_HTTPUTILS_H
