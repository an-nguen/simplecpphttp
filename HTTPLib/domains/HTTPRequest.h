#ifndef REQUEST_H
#define REQUEST_H

#include <iostream>
#include <string>
#include <sstream>
#include <map>
#include <utility>
#include "../enums/HTTP_STATUS.h"
#include "../enums/HTTP_METHOD.h"
#include "../enums/HTTP_PROTOCOL.h"
#include "../utils/HTTPUtils.h"

namespace cpphttp {

    struct Request {
        public:
        std::map<std::string, std::string> headers{};
        std::string body{};
        HTTP_METHOD method{};
        std::string path{};
        HTTP_PROTOCOL_ENUM protocol{};

        Request() = default;
        explicit Request(const std::string& rawHttp) {
            parseRequest(rawHttp);
        }

        HTTP_STATUS parseRequest(const std::string& raw) {
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
                if (requestLine.size() != 3)
                    return BAD_REQUEST;
                this->method = getHTTPMethod(requestLine.at(0));
                if (this->method == NO_METHOD)
                    return METHOD_NOT_ALLOWED;
                this->path = requestLine.at(1);
                this->protocol = getHTTPProtocol(requestLine.at(2).c_str());
                // 6. The remainder of the header write to std::map this->headers
                for (auto &i : headerVec) {
                    auto req_header = HTTPUtils::split(i, ":");
                    if (req_header.size() == 2)
                        headers.emplace(req_header.at(0), req_header.at(1));
                    
                }
                // 7. Write body string to member
                this->body = bodyStr;
            } else {
                return BAD_REQUEST;
            }
            return OK;
        }

        [[nodiscard]] std::string read() const {
            return this->body;
        }

        std::string toString() {
            std::stringstream ss;
            ss << HTTP_METHOD_STR.at(method) << " " << path << " " << HTTP_PROTOCOL_STR.at(protocol) << std::endl;
            for (const auto& [first, second]: headers) {
                ss << first << ": " << second << std::endl;
            }
            ss << std::endl << std::endl;
            ss << body << std::endl;
            return ss.str();
        }

        friend std::ostream& operator<<(std::ostream &out, const Request &request) {
            out << HTTP_METHOD_STR.at(request.method) << " " <<
                request.path << " " << request.protocol << " " << std::endl;
            for (const auto& [first, second] : request.headers) {
                out << first << ": " << second << std::endl;
            }
            out << std::endl << request.body << std::endl;

            return out;
        };
    };
}


#endif // REQUEST_H
