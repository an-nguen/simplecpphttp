//
// Created by an on 08.03.2020.
//

#ifndef CPPHTTP_HTTPRESPONSE_H
#define CPPHTTP_HTTPRESPONSE_H

#include <string>
#include <sstream>
#include "../enums/HTTP_STATUS.h"
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

        void parse(const std::string &raw) {
            // 1. Find separator of HTTP header and HTTP Body
            auto found = raw.find(HTTP_HEADER_BODY_DIVIDER);
            if (found != std::string::npos) {
                // 2. Read header to string
                auto headerStr = raw.substr(0, found + std::strlen(HTTP_HEADER_BODY_DIVIDER));
                // 3. Read body string
                auto bodyStr = raw.substr(found + std::strlen(HTTP_HEADER_BODY_DIVIDER),
                                          (raw.length() - (found + std::strlen(HTTP_HEADER_BODY_DIVIDER))));
                // 4. Split header string to string vector
                auto headerVec = HTTPUtils::split(headerStr, HTTP_NEW_LINE_CHARACTERS);
                // 5. Parse first HTTP header line
                auto requestLine = HTTPUtils::split(headerVec.at(0), " ");
                if (requestLine.size() < 2)
                    return;
                this->protocol = getHTTPProtocol(requestLine.at(0));
                this->status = static_cast<HTTP_STATUS>(std::stoi(requestLine.at(1)));
                this->status_msg = HTTP_STATUS_STR.at(this->status);
                // 6. The remainder of the header write to std::map this->headers
                for (auto &i : headerVec) {
                    auto req_header = HTTPUtils::split(i, ":");
                    if (req_header.size() == 2)
                        headers.emplace(req_header.at(0), req_header.at(1));

                }
                // 7. Write body string to member
                this->body = bodyStr;
            }
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
