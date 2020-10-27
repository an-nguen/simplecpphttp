//
// Created by an on 7/28/20.
//

#ifndef CPPHTTP_TCPCLIENT_H
#define CPPHTTP_TCPCLIENT_H

#include "domains/HTTPRequest.h"
#include "domains/HTTPResponse.h"
#include "exceptions/TCPClientException.h"

#include <string>
#include <memory>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <resolv.h>
#include <netdb.h>

namespace cpphttp {
    class TCPClient {
    private:
        int m_listenFd{};
        int m_epollFd{};
        int m_maxEvents = 1024;
        struct epoll_event m_event{}, *m_events{};
        struct sockaddr_in dest{};
        static constexpr auto BUF_SZ = 1024;
    public:
        TCPClient() = default;
        ~TCPClient() = default;

        std::string hostnameToIp(const std::string &host) {
            struct addrinfo hints{};
            struct addrinfo *serverInfo;
            struct addrinfo *p;
            struct sockaddr_in *h;
            int err;

            char ip[128];

            memset(&hints, 0, sizeof hints);
            hints.ai_family = AF_UNSPEC; // use AF_INET6 to force IPv6
            hints.ai_socktype = SOCK_STREAM;
            if ((err = getaddrinfo(host.c_str(), "http", &hints, &serverInfo)) != 0) {
                throw std::runtime_error("getaddrinfo: %s\n" + std::string(gai_strerror(err)));
            }
            // loop through all the results and connect to the first we can
            for(p = serverInfo; p != nullptr; p = p->ai_next)
            {
                h = (struct sockaddr_in *) p->ai_addr;
                strcpy(ip , inet_ntoa( h->sin_addr ) );
            }
            freeaddrinfo(serverInfo); // all done with this structure

            return std::string(ip);
        }

        std::unique_ptr<Response, std::default_delete<Response>>
        send(HTTP_METHOD http_method, const std::string &host, unsigned short port, const std::string &msg) {
            std::string raw{};
            // 1. Open socket
            this->m_listenFd = socket(AF_INET, SOCK_STREAM, 0);
            if (this->m_listenFd == -1) {
                throw TCPClientException("socket(..): ", std::string(std::strerror(errno)));
            }
            dest.sin_family = AF_INET;
            dest.sin_port = htons(port);
            // 2. Convert host to IP byte sequence
            auto ip = hostnameToIp(host);
            if (inet_pton(AF_INET, ip.c_str(), &dest.sin_addr.s_addr) == 0) {
                throw TCPClientException("inet_pton(..): " + std::string(std::strerror(errno)));
            }
            // 3. Connect to remote IP
            if (connect(this->m_listenFd, (struct sockaddr*)&dest, sizeof(dest)) != 0 ) {
                if(errno != EINPROGRESS) {
                    throw TCPClientException("connect(..): " + std::string(std::strerror(errno)));
                }
            }
            // 4. Write request to socket
            auto request = std::make_unique<Request>();
            request->method = http_method;
            request->body.assign(msg);
            request->path.assign("/");
            request->protocol = HTTP_1_1;
            request->headers.emplace("Host", host);
            auto n = write(m_listenFd, request->toString().c_str(), request->toString().size());
            if (n == -1) {
                throw std::runtime_error("write error: " + std::to_string(errno));
            }
            int cnt = 0;
            char buf[BUF_SZ];
            while((n = read(this->m_listenFd, buf, BUF_SZ)) > 0) {
                raw.append(buf);
                cnt += n;
            }
            auto response = std::make_unique<Response>();
            response->parse(raw);
            return response;
        }
    };
}

#endif //CPPHTTP_TCPCLIENT_H
