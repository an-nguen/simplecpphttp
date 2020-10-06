//
// Created by an on 7/28/20.
//

#ifndef CPPHTTP_CLIENT_H
#define CPPHTTP_CLIENT_H

#include "HTTPRequest.h"

#include <string>
#include <memory>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <resolv.h>

namespace CPPHTTP {
    class Client {
    private:
        int m_listenFd{};
        int m_epollFd{};
        int m_maxEvents = 1024;
        int m_bufferSize = 4096;
        struct epoll_event m_event{}, *m_events{};
        struct sockaddr_in dest{};
        char *m_buffer;
    public:
        Client() = default;

        void initEpollFd() {
            this->m_epollFd = epoll_create(1);
            if (this->m_epollFd == -1) {
                perror("epoll_create(...) error: ");
                throw std::runtime_error("failed to create epoll file descriptor");
            }
            this->m_event.events = EPOLLIN;
            this->m_event.data.fd = this->m_listenFd;
            if (epoll_ctl(this->m_epollFd, EPOLL_CTL_ADD, this->m_listenFd, &this->m_event) != 0) {
                perror("epoll_ctl(...) error:");
                throw std::runtime_error("epoll_ctl(...) failed!");
            }
        }

        explicit Client(int max_events, int bufferSize) : m_maxEvents(max_events), m_bufferSize(bufferSize) {
            initEpollFd();
            this->m_events = new epoll_event[m_maxEvents];
            this->m_buffer = new char[bufferSize];
        }

        ~Client() {
            delete this->m_events;
            delete this->m_buffer;
        }

        std::string & send(HTTP_METHOD http_method, const std::string &host, unsigned short port, long timeout, const std::string &msg) {
            int nEvent;
            std::string raw;
            struct epoll_event event{};

            dest.sin_family = AF_INET;
            dest.sin_port = htons(port);
            if (inet_pton(AF_INET, host.c_str(), &dest.sin_addr.s_addr) == 0) {
                perror("inet_pton failed!");
                throw std::runtime_error("inet_pton failed.");
            }
            if (connect(this->m_listenFd, (struct sockaddr*)&dest, sizeof(dest)) != 0 ) {
                if(errno != EINPROGRESS) {
                    perror("Connect ");
                    throw std::runtime_error("failed to connect");
                }
            }
            nEvent = epoll_wait(this->m_epollFd, m_events, this->m_maxEvents, timeout);
            for (auto i = 0; i < nEvent; i++) {
                if (this->m_events[i].events & EPOLLIN) {
                    std::cout << "Socket " + std::to_string(this->m_events[i].data.fd) + " connected\n";
                }
            }

            std::shared_ptr<Request> request;
            request->method = http_method;
            request->body.assign(msg);
            auto n = write(m_listenFd, request->body.c_str(), request->body.size());
            if (n == -1) {
                throw std::runtime_error("write error: " + std::to_string(errno));
            }

            nEvent = epoll_wait(m_epollFd, m_events, m_maxEvents, timeout);
            for (auto i = 0; i < nEvent; i++) {
                if (this->m_events[i].events & EPOLLIN) {
                    int cnt = 0;
                    while((n = read(this->m_listenFd, this->m_buffer, this->m_bufferSize)) ) {
                        raw.append(m_buffer);
                        cnt += n;
                    }
                    if (n == -1) {
                        if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
                            perror("read: ");
                            if (epoll_ctl(m_epollFd, EPOLL_CTL_DEL, m_listenFd, &event) == -1) {
                                perror("[PERROR] epoll_ctl: connection");
                                break;
                            }
                        }
                    } else if (n == 0) {
                        if (epoll_ctl(m_epollFd, EPOLL_CTL_DEL, m_listenFd, &event) == -1) {
                            perror("[PERROR] epoll_ctl: connection");
                            break;
                        }
                        break;
                    }
                }
            }

            return raw;
        }
    };
}

#endif //CPPHTTP_CLIENT_H
