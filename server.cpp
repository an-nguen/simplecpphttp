#include <string>
#include <random>

#include "HTTPLib/HTTPHandler.h"
#include "HTTPLib/Server.h"

#include "Logger/SimpleLogger.h"

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

int main() {
    using rapidjson::Document;
    using rapidjson::StringBuffer;
    using rapidjson::Writer;
    using rapidjson::Value;
    StringBuffer buffer;
    // Create logger for http server
    SimpleLogger logger;

    // Create HTTP handler
    cpphttp::HTTPHandler httpHandler(16, logger);

    // endpoint '/' - return { "hello" : "world" }
    httpHandler.addResource("/", cpphttp::GET, [&](std::shared_ptr<cpphttp::Request> &req, std::shared_ptr<cpphttp::Response> &resp) {
        resp->headers.emplace("Content-Type", "application/json");
        Document d(rapidjson::kObjectType);
        Value v(rapidjson::kStringType);
        v.SetString("world", std::strlen("world"), d.GetAllocator());
        d.AddMember("hello", v, d.GetAllocator());
        buffer.Clear();
        Writer<StringBuffer> writer(buffer);
        d.Accept(writer);
        resp->body = std::string(buffer.GetString());

    });
    // endpoint '/random' - generate random number and return in json response
    httpHandler.addResource("/random", cpphttp::GET, [&](std::shared_ptr<cpphttp::Request> &req, std::shared_ptr<cpphttp::Response> &resp) {
        resp->headers.emplace("Content-Type", "application/json");
        Document d(rapidjson::kObjectType);
        auto &allocator = d.GetAllocator();
        if (req->method == cpphttp::GET) {
            std::random_device r;
            std::default_random_engine e1(r());
            std::uniform_int_distribution<int> uniform_dist(1, 100);
            auto mean = uniform_dist(e1);
            buffer.Clear();
            Writer<StringBuffer> writer(buffer);
            d.AddMember("num", Value().SetInt(mean), allocator);
            d.Accept(writer);
            resp->body = std::string(buffer.GetString());
        } else {
            buffer.Clear();
            Writer<StringBuffer> writer(buffer);
            d.AddMember("error", "method not implemented", allocator);
            d.Accept(writer);
            resp->body.assign(std::string(buffer.GetString()));
        }
    });

    // Create server instance
    cpphttp::Server serverSocket(8080, 16386, 16386, 1, httpHandler, logger);

    // Start server
    serverSocket.listenAndServe();
    return 0;
}
