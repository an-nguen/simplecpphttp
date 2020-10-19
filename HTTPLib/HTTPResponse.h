//
// Created by an on 08.03.2020.
//

#ifndef CPPHTTP_HTTPRESPONSE_H
#define CPPHTTP_HTTPRESPONSE_H

#include <string>
#include <sstream>
#include "HTTP_STATUS.h"
#include "rapidjson/document.h"

namespace cpphttp {
    struct Response {
        // Status line
        HTTP_PROTOCOL_ENUM protocol;
        HTTP_STATUS status;
        std::string status_msg;
        // Response headers
        std::map <std::string, std::string> headers{};
        // Response body
        std::string body;
        // Extra members


        void write(const std::string &msg) {
            this->body.assign(msg);
        }

        std::string toString() {
            std::stringstream ss;
            ss << HTTP_PROTOCOL_STR[protocol] << " " << status << " " << status_msg << std::endl;
            for (const auto& [first, second]: headers) {
                ss << first << ": " << second << std::endl;
            }
            ss << std::endl << std::endl;
            ss << body << std::endl;
                return ss.str();
        }
    };
}
#endif //CPPHTTP_HTTPRESPONSE_H
