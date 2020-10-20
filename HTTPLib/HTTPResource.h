//
// Created by an on 7/29/20.
//

#ifndef CPPHTTP_HTTPRESOURCE_H
#define CPPHTTP_HTTPRESOURCE_H

#include <functional>
#include <array>

#include "HTTPRequest.h"
#include "HTTPResponse.h"

namespace cpphttp {
    struct Resource {
    private:
        std::array<std::function<void(std::shared_ptr<Request> &, std::shared_ptr<Response> &)>, 11> m_methods{};
    public:
        Resource() = default;
        explicit Resource(HTTP_METHOD httpMethod,
                          const std::function<void(std::shared_ptr<Request> &, std::shared_ptr<Response> &)>& func) {
            this->addMethod(httpMethod, func);
        }

        void addMethod(HTTP_METHOD httpMethod,
                       const std::function<void(std::shared_ptr<Request> &, std::shared_ptr<Response> &)> func) {
            m_methods.at(httpMethod) = func;
        }

        [[nodiscard]] bool is_method_initialized(HTTP_METHOD httpMethod) const {
            return this->m_methods.at(httpMethod) != nullptr;
        }

        void call(HTTP_METHOD httpMethod, std::shared_ptr<Request> &request, std::shared_ptr<Response> &response) {
            m_methods.at(httpMethod)(request, response);
        }
    };
}

#endif //CPPHTTP_HTTPRESOURCE_H
