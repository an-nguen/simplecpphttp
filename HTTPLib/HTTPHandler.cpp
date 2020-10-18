#include "HTTPHandler.h"
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include <unistd.h>
#include <cstring>
#include <thread>
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#define BUF_SZ 32000

namespace CPPHTTP {

    void HTTPHandler::handle(int epoll_fd, struct epoll_event &event, int conn_fd) {
        using rapidjson::Document;
        using rapidjson::kObjectType;
        using rapidjson::StringBuffer;
        using rapidjson::Writer;
        using rapidjson::Value;

        bool isDone = false, isShouldClose = false;
        std::string raw{};
        char buffer[BUF_SZ];

        while (!isDone) {
            std::shared_ptr<Request> request(new Request());
            std::shared_ptr<Response> response(new Response());
            // Fill zero buffer
            bzero(buffer, BUF_SZ);
            int n, cnt = 0;
            while((n = read(conn_fd, buffer, m_read_size)) > 0) {
                raw.append(buffer);
                cnt += n;
            }
            if (n == -1) {
                if (!(errno == EAGAIN || errno == EWOULDBLOCK)) {
                    perror("read");
                    isShouldClose = true;
                    if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn_fd, &event) == -1) {
                        perror("epoll_ctl");
                        break;
                    }
                }
                isDone = true;
            } else if (n == 0) {
                isShouldClose = isDone = true;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn_fd, &event) == -1) {
                    perror("epoll_ctl");
                    break;
                }
                break;
            }
            if (cnt > 0) {
                HTTP_STATUS res = request->parseRequest(raw);

                Document d(kObjectType);
                auto &allocator = d.GetAllocator();
                StringBuffer buffer;
                Value v;
                buffer.Clear();
                Writer<StringBuffer> writer(buffer);

                response->protocol = HTTP_PROTOCOL_ENUM::HTTP_1_1;
                if (res == OK) {
                    if (this->m_resources.contains(request->path)) {
                        if (this->m_resources[request->path].is_method_initialized(request->method)) {
                            response->status = HTTP_STATUS::OK;
                            response->status_msg = HTTP_STATUS_STR.at(HTTP_STATUS::OK);
                            response->headers.emplace("Server", "CPPHTTP");
                            response->headers.emplace("Access-Control-Allow-Origin", "*");

                            this->m_resources[request->path].call(request->method, request.get(), response.get());
                            std::cout << getHTTPMethodStr(request->method) << "  ->  [\"" << request->path << "\"]" << std::endl;

                            for (auto &handler: this->m_queue_handlers_after) {
                                handler(request.get(), response.get());
                            }
                        } else {
                            setErrorResponse(d, allocator, buffer, writer, response, v, HTTP_STATUS::NOT_IMPLEMENTED);
                        }
                    } else {
                        setErrorResponse(d, allocator, buffer, writer, response, v, HTTP_STATUS::NOT_FOUND);
                    }
                } else if (res == BAD_REQUEST) {
                    setErrorResponse(d, allocator, buffer, writer, response, v, HTTP_STATUS::BAD_REQUEST);
                }

                response->status_msg = HTTP_PROTOCOL_STR[response->protocol];
                if (!response->headers.contains("Content-Type"))
                    response->headers.emplace("Content-Type", "application/json");
                response->headers.emplace("Connection", "close");
                n = write(conn_fd, response->toString().c_str(), response->toString().size());
                if (n == 0 || n == -1) {
                    std::cerr << "failed to write msg" << std::endl;
                    isDone = true;
                }

                isShouldClose = true;
            }
        }

        if (isShouldClose) {
           close(conn_fd);
        }

    }

    void HTTPHandler::setErrorResponse(rapidjson::Document &d,
                                       rapidjson::MemoryPoolAllocator<> &allocator,
                                       const rapidjson::StringBuffer &buffer,
                                       rapidjson::Writer<rapidjson::StringBuffer> &writer,
                                       std::shared_ptr<Response> &response,
                                       rapidjson::Value &v,
                                       HTTP_STATUS status) {
        response->status = status;
        v.SetString(HTTP_STATUS_STR.at(status), allocator);
        d.AddMember("error", v, allocator);
        d.Accept(writer);
        response->body = std::string(buffer.GetString());
    }

    void HTTPHandler::addResource(const std::string& path,
                                  HTTP_METHOD httpMethod,
                                  const std::function<void(Request *, Response *)>& func) {
        CPPHTTP::Resource resource(httpMethod, func);
        this->m_resources.emplace(path, resource);
    }

    void HTTPHandler::addHandlerAfter(const std::function<void(Request *, Response *)>& func) {
        this->m_queue_handlers_after.push_back(func);
    }

    void HTTPHandler::setReadSize(unsigned int rs) {
        this->m_read_size = rs;
    }

    [[maybe_unused]] constexpr unsigned int HTTPHandler::getReadSize() const {
        return this->m_read_size;
    }
}
