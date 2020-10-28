//
// Created by an on 10/27/20.
//

#ifndef CPPHTTP_HTTP_CONCEPTS_H
#define CPPHTTP_HTTP_CONCEPTS_H

#include <string>
#include <fmt/core.h>

#include "../abstractions/AbstractHandler.h"

namespace cpphttp {
    template <class T>
    concept SameString = std::is_same<T, std::basic_string<char>>::value ||
    std::is_same<T, const char *>::value || std::is_same<T, char *>::value;

    template <class T>
    concept DerivedAbstractHandler = std::is_base_of<AbstractHandler, T>::value;
}

#endif //CPPHTTP_HTTP_CONCEPTS_H
