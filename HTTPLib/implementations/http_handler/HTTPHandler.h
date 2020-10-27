#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include <sys/epoll.h>
#include <unistd.h>

#include <string>
#include <map>
#include <functional>
#include <mutex>
#include <memory>
#include <cstring>

#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <exceptions/HTTPHandlerException.h>

#include "domains/HTTPRequest.h"
#include "domains/HTTPResponse.h"
#include "HTTPResource.h"
#include "../../../Logger/AbstractLogger.h"
#include "abstractions/AbstractHandler.h"

namespace cpphttp {
#define BUF_SZ 32000
    using std::runtime_error;
    using rapidjson::Document;
    using rapidjson::kObjectType;
    using rapidjson::StringBuffer;
    using rapidjson::Writer;
    using rapidjson::Value;
    using rapidjson::MemoryPoolAllocator;

    template <class L> requires logs::DerivedAbstractLogger<L>
    class HTTPHandler: public AbstractHandler {
    public:
        HTTPHandler() = default;

        explicit HTTPHandler(unsigned int readSize, L logger) : m_read_size(readSize), m_logger(logger) {}

        [[maybe_unused]] [[nodiscard]] constexpr unsigned int getReadSize() const {
            return this->m_read_size;
        }
        [[maybe_unused]] void setReadSize(unsigned int rs) {
            this->m_read_size = rs;
        }

        void handle(int epollFd, struct epoll_event &event, int connFd) override {
            bool isDone = false, isShouldClose = false;
            std::string raw{};
            std::shared_ptr<Request> request(new Request());
            std::shared_ptr<Response> response{new Response()};

            Document d(rapidjson::kObjectType);
            auto &allocator = d.GetAllocator();
            StringBuffer stringBuffer;
            stringBuffer.Clear();
            Writer<StringBuffer> writer(stringBuffer);
            Value v;

            try {
                while (!isDone) {
                    ssize_t cnt = 0;

                    /* 1. read data from connection file descriptor to raw */
                    auto nBytes = readConnection(connFd, raw, cnt);
                    if (nBytes == -1) {
                        if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
                            m_logger.error("read: " + std::string(std::strerror(errno)));
                            isShouldClose = true;
                            if (epoll_ctl(epollFd, EPOLL_CTL_DEL, connFd, &event) == -1) {
                                throw HTTPHandlerException("epoll_ctl: " + std::string(std::strerror(errno)));
                            }
                        }
                        isDone = true;
                    } else if (nBytes == 0) {
                        if (epoll_ctl(epollFd, EPOLL_CTL_DEL, connFd, &event) == -1)
                            throw HTTPHandlerException("epoll_ctl" + std::string(std::strerror(errno)));
                    }
                    if (cnt > 0) {
                        /* parse/handle acquired data to Response struct instance */
                        handleResponse(raw, request, response,
                                                  d, allocator, stringBuffer, writer, v);
                        response->headers.emplace("TCPServer", "cpphttp");
                        response->headers.emplace("Access-Control-Allow-Origin", "*");
                        /* 3. write data to connection file descriptor */
                        nBytes = writeConnection(connFd, response);
                        if (nBytes == 0 || nBytes == -1) {
                            m_logger.error("failed to write msg");
                            isDone = true;
                        }
                        if (!response->headers.contains("Content-Type"))
                            response->headers.emplace("Content-Type", "application/json");
                        response->headers.emplace("Connection", "close");

                        isShouldClose = true;
                    }
                }
                if (isShouldClose) {
                    close(connFd);
                }
            } catch (std::exception &e) {
                m_logger.error(e.what());
                response->headers.emplace("TCPServer", "cpphttp");
                response->headers.emplace("Access-Control-Allow-Origin", "*");
                response->protocol = HTTP_1_1;
                setErrorResponse(d, allocator, stringBuffer, writer, response, v, INTERNAL_SERVER_ERROR);
                response->headers.emplace("Content-Type", "application/json");
                response->headers.emplace("Connection", "close");
            }
        }


        [[maybe_unused]] void addHandlerAfter(const std::function<void(std::shared_ptr<Request> &, std::shared_ptr<Response> &)>& func) {
            this->m_queue_handlers_after.push_back(func);
        }
        void addResource(const std::string& path,
                         HTTP_METHOD httpMethod,
                         const std::function<void(std::shared_ptr<Request> &, std::shared_ptr<Response> &)>& func) {
            cpphttp::Resource resource(httpMethod, func);
            this->m_resources.emplace(path, resource);
        }
    private:
        std::vector<std::function<void(std::shared_ptr<Request> &, std::shared_ptr<Response> &)>> m_queue_handlers_after{};
        std::map<std::string, Resource> m_resources{};
        unsigned int m_read_size{};
        L m_logger{};

        static void setErrorResponse(rapidjson::Document &d,
                                     rapidjson::MemoryPoolAllocator<> &allocator,
                                     const rapidjson::GenericStringBuffer<rapidjson::UTF8<>> &buffer,
                                     rapidjson::Writer<rapidjson::StringBuffer> &writer,
                                     std::shared_ptr<Response> &response,
                                     rapidjson::Value &v,
                                     HTTP_STATUS status) {
            response->status = status;
            response->status_msg = HTTP_STATUS_STR.at(status);
            v.SetString(HTTP_STATUS_STR.at(status).c_str(), allocator);
            d.AddMember("error", v, allocator);
            d.Accept(writer);
            response->body = std::string(buffer.GetString());
        }

        void handleResponse(const std::string &raw, std::shared_ptr<Request> &request, std::shared_ptr<Response> &response,
                                                  Document &d, MemoryPoolAllocator<> &allocator,
                                                  StringBuffer &stringBuffer, Writer<StringBuffer> &writer, Value &v ) {
            HTTP_STATUS res = request->parseRequest(raw);

            response->protocol = HTTP_1_1;
            if (res != OK) {
                setErrorResponse(d, allocator, stringBuffer, writer, response, v, res);
            } else if (!m_resources.contains(request->path)){
                setErrorResponse(d, allocator, stringBuffer, writer, response, v, NOT_FOUND);
            } else if (!m_resources[request->path].is_method_initialized(request->method)) {
                setErrorResponse(d, allocator, stringBuffer, writer, response, v, NOT_IMPLEMENTED);
            } else {
                response->status = OK;
                response->status_msg = HTTP_STATUS_STR.at(OK);

                m_resources[request->path].call(request->method, request, response);
                m_logger.info(std::string(getHTTPMethodStr(request->method)) + "  ->  [\"" + request->path + "\"]");

                for (auto &handler: m_queue_handlers_after) {
                    try {
                        handler(request, response);
                    } catch (std::exception &e) {
                        std::cerr << e.what() << std::endl;
                    }
                }
            }

            response->status_msg = HTTP_PROTOCOL_STR[response->protocol];
        }

        ssize_t readConnection(int connectionFd, std::string &raw, ssize_t &cnt) {
            // Fill zero buffer
            char buffer[BUF_SZ];
            bzero(buffer, BUF_SZ);
            ssize_t n = 0;
            while((n = read(connectionFd, buffer, m_read_size)) > 0) {
                raw.append(buffer);
                cnt += n;
            }

            return n;
        }

        ssize_t writeConnection(int connectionFd, std::shared_ptr<Response> &response) {
            return write(connectionFd, response->toString().c_str(), response->toString().size());
        }
    };
}


#endif // HTTPHANDLER_H
