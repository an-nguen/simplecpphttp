//
// Created by an on 7/29/20.
//

#ifndef CPPHTTP_HTTPRESOURCE_H
#define CPPHTTP_HTTPRESOURCE_H

#include <functional>
#include <array>

#include "HTTPRequest.h"
#include "HTTPResponse.h"


namespace CPPHTTP {
#define SET_FUNC(method, func) this->HTTP_METHOD_STR(method) =

    struct Resource {
    private:
        std::array<std::function<void(Request *, Response *)>, 11> m_methods{};
    public:
        Resource() = default;
        explicit Resource(HTTP_METHOD httpMethod, const std::function<void(Request *, Response *)> &func) {
            this->addMethod(httpMethod, func);
        }

        void addMethod(HTTP_METHOD httpMethod, const std::function<void(Request *, Response *)> &func) {
            m_methods.at(httpMethod) = func;
        }

        [[nodiscard]] bool is_method_initialized(HTTP_METHOD httpMethod) const {
            return this->m_methods.at(httpMethod) != nullptr;
        }

        void call(HTTP_METHOD httpMethod, Request *request, Response *response) {
            m_methods.at(httpMethod)(request, response);
        }
    };
}

#endif //CPPHTTP_HTTPRESOURCE_H
