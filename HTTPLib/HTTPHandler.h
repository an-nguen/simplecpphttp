#ifndef HTTPHANDLER_H
#define HTTPHANDLER_H

#include <string>
#include <map>
#include <functional>
#include <mutex>
#include "HTTPRequest.h"
#include "HTTPResponse.h"
#include "HTTPResource.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <sys/epoll.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <memory>

namespace CPPHTTP {
    class HTTPHandler {
    public:
        HTTPHandler() = default;

        explicit HTTPHandler(unsigned long readSize) : m_read_size(readSize) {}

        [[maybe_unused]] [[nodiscard]] constexpr unsigned int getReadSize() const;
        void setReadSize(unsigned int );

        void handle(int epoll_fd, struct epoll_event &event, int conn_fd);
        void addHandlerAfter(const std::function<void(Request *, Response *)>& func);
        void addResource(const std::string& path,
                         HTTP_METHOD httpMethod,
                         const std::function<void(Request *, Response *)>& func);
    private:
        std::vector<std::function<void(Request *, Response *)>> m_queue_handlers_after{};
        std::map<std::string, Resource> m_resources{};
        unsigned int m_read_size{};

        static void setErrorResponse(rapidjson::Document &d,
                              rapidjson::MemoryPoolAllocator<> &allocator,
                              const rapidjson::GenericStringBuffer<rapidjson::UTF8<>> &buffer,
                              rapidjson::Writer<rapidjson::StringBuffer> &writer,
                              std::shared_ptr<Response> &response,
                              rapidjson::Value &v,
                              HTTP_STATUS status) ;
    };
}


#endif // HTTPHANDLER_H
