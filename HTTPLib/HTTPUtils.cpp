//
// Created by an on 09.02.2020.
//

#include "HTTPUtils.h"

std::vector<std::string> cpphttp::HTTPUtils::split(const std::string& str = "", const std::string& divider = "\n") {
    std::vector <std::string> vecStr {};
    // Make copy of original string
    std::string copy(str);

    // Push substrings to vecStr and delete in copy of original string
    while (!copy.empty()) {
        auto found = copy.find_first_of(divider);
        if (found != std::string::npos) {
            vecStr.push_back(copy.substr(0, found));
            copy.erase(0, found + divider.length());
        } else {
            vecStr.push_back(copy.substr(0, copy.length()));
            break;
        }
    }

    return vecStr;
}
