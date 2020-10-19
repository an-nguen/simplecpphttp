#ifndef SOCKET_SERVER_H
#define SOCKET_SERVER_H

#include <cstdio>           /* perror */
#include <cstdlib>
#include <sys/types.h>
#include <sys/socket.h>		/* socket */
#include <netinet/in.h>
#include <unistd.h>
#include <thread>
#include <mutex>
#include <utility>
#include <sys/epoll.h>
#include <fcntl.h>
#include <netdb.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <list>

#include "HTTPHandler.h"
#include "../Logger/AbstractLogger.h"

using std::thread;

namespace cpphttp {
    class TCPConnection {
    private:
        int connection;
    public:
        [[nodiscard]] int getConnection() const {
            return connection;
        }

        void setConnection(int connectionDescriptor) {
            this->connection = connectionDescriptor;
        }
    };

    template <class L> requires logs::DerivedAbstractLogger<L>
    class Server {
    private:
        // Main file descriptor for listen
        int m_listen_fd{};
        // File descriptor for epoll instance
        // Event and thread count that should be defined through class constructor
        int m_max_events_count{};
        unsigned int m_thread_count{};
        // Server port
        unsigned short m_port{};
        unsigned int m_backlog{};
        // Threads
        std::vector<thread> m_threads{};
        // Struct or class that handle incoming request
        cpphttp::HTTPHandler<L> m_http_handler{};
        std::map<int, TCPConnection> connections{};
        L m_logger;

        void initSocketFileDescriptor(unsigned short port, unsigned int backlog) {
            int err;

            struct addrinfo hints{};
            struct addrinfo *res{};

            hints.ai_family = AF_UNSPEC;
            hints.ai_socktype = SOCK_STREAM;
            hints.ai_flags = AI_PASSIVE;

            // return addrinfo that contain Internet address
            err = getaddrinfo(nullptr, std::to_string(port).c_str(), &hints, &res);
            if (err != 0) {
                if (err == EAI_SYSTEM)
                    throw std::runtime_error("getaddrinfo" + std::string(std::strerror(errno)));
                else
                    throw std::runtime_error("error in getaddrinfo: " + std::string(gai_strerror(err)) + "\n");
            }
            for (auto p = res; p != nullptr; p = p->ai_next) {
                int opt = 1;

                /* Socket create endpoint for communication without name
                 * in namespace (family address) and return file descriptor
                 */
                if ((this->m_listen_fd = socket(p->ai_family, p->ai_socktype, 0)) < 0)
                    throw std::runtime_error("socket(..)" + std::string(std::strerror(errno)));

                /* Set options to file descriptor socket.  */
                if (setsockopt(this->m_listen_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)) < 0)
                    throw std::runtime_error("setsockopt(..)" + std::string(std::strerror(errno)));

                /* Attach local address to socket file descriptor with address length. */
                if (bind(this->m_listen_fd, p->ai_addr, p->ai_addrlen) < 0)
                    throw std::runtime_error("bind(..)" + std::string(std::strerror(errno)));
            }

            /* Mark a connection-mode socket.
             * First parameter - socket file descriptor
             * Second parameter - backlog, max connection queued
             * (listen(2))
             */
            if (listen(this->m_listen_fd, backlog) < 0)
                throw std::runtime_error("listen(..)" + std::string(std::strerror(errno)));

            freeaddrinfo(res);
        }

        static int createEpollFd(int listen_fd, struct epoll_event &event) {
            // Create file descriptor for epoll instance
            auto epoll_fd = epoll_create1(0);
            if (epoll_fd < 0)
                throw std::runtime_error("failed to create epoll instance" + std::string(std::strerror(errno)));

            event.events = EPOLLIN;
            event.data.fd = listen_fd;
            if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, listen_fd, &event) == -1)
                throw std::runtime_error("failed change instance (epoll_ctl(..)) " + std::string(std::strerror(errno)));

            return epoll_fd;
        }

        static int setNonBlock(int fd) {
            int flags;
            flags = fcntl(fd, F_GETFL, 0);
            flags |= O_NONBLOCK;
            if (fcntl(fd, F_SETFL, flags) == -1) {
                perror("fcntl: fd");
                return -1;
            }
            return 0;
        }

        void socketThreadProcess(int epoll_fd, struct epoll_event &event, int listen_fd) {
            m_http_handler.handle(epoll_fd, event, listen_fd);
        }

        void acceptConnection(epoll_event &event, int epoll_fd) {
            struct sockaddr in_addr{};
            socklen_t inLength = sizeof in_addr;
            char hBuf[NI_MAXHOST], sBuf[NI_MAXSERV];
            TCPConnection connection{};
            /* accept()
             * Extract the first connection on the queue of pending connections.
             * Create new socket with the same socket type protocol and address family as the specified socket,
             * and allocate a new file descriptor for that socket.
             */
            connection.setConnection(accept(m_listen_fd, &in_addr, &inLength));
            if (connection.getConnection() < 0) {
                throw std::runtime_error("accept(..) failed");
            } else {

                if (getnameinfo(&in_addr, inLength, hBuf, sizeof hBuf, sBuf, sizeof sBuf,
                                NI_NUMERICHOST | NI_NUMERICSERV) < 0)
                    throw std::runtime_error("getnameinfo(..): " + std::string(std::strerror(errno)));

                // Set non blocking
                setNonBlock(connection.getConnection());
                event.events = EPOLLIN | EPOLLET | EPOLLHUP;
                event.data.fd = connection.getConnection();

                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, connection.getConnection(), &event) < 0)
                    throw std::runtime_error("epoll_ctl(..) failed:" + std::string(std::strerror(errno)));

                if (!this->connections.contains(connection.getConnection()))
                    this->connections.try_emplace(connection.getConnection(), connection);
//
//                auto it = connections.begin();
//                while (it != connections.end()) {
//                    std::cout << it->first << std::endl;
//                    it++;
//                }
            }
        }

        void eventLoop() {
            try {
                struct epoll_event event{};
                std::vector<struct epoll_event> events(this->m_max_events_count);
                int nEvent;
                // Create epoll instance (file descriptor)
                auto epoll_fd = createEpollFd(this->m_listen_fd, event);
                if (epoll_fd < 0 || this->m_listen_fd < 0)
                    throw std::runtime_error("m_epollFd or m_listenFd < 0!\n");

                while (true) {
                    nEvent = epoll_wait(epoll_fd, events.data(), m_max_events_count, -1);
                    if (nEvent == -1)
                        throw std::runtime_error("epoll_wait(...): " + std::string(std::strerror(errno)));

                    for (unsigned long i = 0; i < nEvent; ++i) {
                        if ((events[i].events & EPOLLERR) || (events[i].events & EPOLLHUP)) {
                            close(events[i].data.fd);
                            continue;
                        } else if (events[i].data.fd == m_listen_fd) {
                            // New connection
                            try {
                                acceptConnection(event, epoll_fd);
                            } catch (std::runtime_error &error) {
                                m_logger.error(error.what());
                            }
                        } else {
                            socketThreadProcess(epoll_fd, event, events[i].data.fd);
                        }
                    }
                }
            } catch (std::exception &e) {
                m_logger.error(e.what());
                eventLoop();
            }
        }

    public:
        Server() = default;

        explicit Server(unsigned short port = 8080,
                        unsigned int backlog = 1024,
                        int max_events = 16386,
                        unsigned int thread_count = 32,
                        cpphttp::HTTPHandler<L> httpHandler = {},
                        L &logger = nullptr)
                : m_port(port), m_backlog(backlog),
                  m_max_events_count(max_events),
                  m_thread_count(thread_count),
                  m_http_handler(std::move(httpHandler)),
                  m_logger(logger) {
            init();
        }

        void init() {
            try {
                initSocketFileDescriptor(this->m_port, this->m_backlog);
            } catch (std::exception &e) {
                m_logger.fatal(e.what());
                throw;
            }
            setNonBlock(m_listen_fd);
            this->m_threads.resize(m_thread_count);
        }

        ~Server() = default;

        void listenAndServe() {
            std::stringstream s;
            s << "Ready to listen on port " << m_port << ".\n";
            m_logger.info(s.str().c_str());
            for (auto &thr: this->m_threads) {
                thr = thread(&Server::eventLoop, this);
                thr.join();
            }
        }
    };
}

#endif // SOCKET_SERVER_H
