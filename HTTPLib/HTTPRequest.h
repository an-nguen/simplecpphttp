#ifndef REQUEST_H
#define REQUEST_H

#include <iostream>
#include <string>
#include <map>
#include <utility>
#include "HTTPUtils.h"
#include "HTTP_STATUS.h"
#include "HTTP_METHOD.h"
#include "HTTP_PROTOCOL.h"

namespace cpphttp {
    constexpr const char HTTP_HEADER_BODY_DIVIDER[] = "\r\n\r\n";
    constexpr const char HTTP_NEW_LINE_CHARACTERS[] = "\r\n";

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
            auto found = raw.find(HTTP_HEADER_BODY_DIVIDER);
            if (found != std::string::npos) {
                auto headerStr = raw.substr(0, found + std::strlen(HTTP_HEADER_BODY_DIVIDER));
                auto bodyStr = raw.substr(found + std::strlen(HTTP_HEADER_BODY_DIVIDER),
                                          (raw.length() - (found + std::strlen(HTTP_HEADER_BODY_DIVIDER))));
                auto headerVec = HTTPUtils::split(headerStr, HTTP_NEW_LINE_CHARACTERS);

                auto reqLine = HTTPUtils::split(headerVec.at(0), " ");
                if (reqLine.size() != 3)
                    return BAD_REQUEST;
                this->method = getHTTPMethod(reqLine.at(0));
                if (this->method == NO_METHOD)
                    return METHOD_NOT_ALLOWED;
                this->path = reqLine.at(1);
                this->protocol = getHTTPProtocol(reqLine.at(2).c_str());

                for (auto i = headerVec.begin() + 1; i != headerVec.end() - 1; i++) {
                    auto req_header = HTTPUtils::split(*i, ":");
                    if (req_header.size() == 2)
                        headers.emplace(req_header.at(0), req_header.at(1));
                    
                }
                this->body = bodyStr;
            } else {
                return BAD_REQUEST;
            }
            return OK;
        }

        [[nodiscard]] std::string read() const {
            return this->body;
        }

        friend std::ostream& operator<<(std::ostream &out, const Request &request) {
            out << HTTP_METHOD_STR[request.method] << " " <<
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
