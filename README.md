# Simple CPPHTTP
Simple HTTP server implementation using linux epoll, sockets, STL library.  
## Quick start
### HTTP Server:
- Create logger and HTTP Handler


```
    #include "Logger/SimpleLogger.h"
    #include "implementations/http_handler/HTTPHandler.h"
    
    ...
    logs::SimpleLogger logger;
    // Create HTTP handler
    cpphttp::HTTPHandler httpHandler(16 // read size of raw response
                                     , logger); // logger
    ...
    
```
- Add endpoint to handler
    
    
```
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
    
```

- Create TCPServer
   
   
```
    ...
    // Create server instance
    cpphttp::TCPServer server(8080, 16386, 16386, 1, httpHandler, logger);
    ...
```
    
- Run TCPServer method listenAndServe()


```
    ...
    // Start server
    server.listenAndServe();
    ...
```

### HTTP Client
- Create TCPClient

    
```
    #include "HTTPLib/TCPClient.h"
    
    ...
    TCPClient client;
    
```
    
- Send HTTP request and get cpphttp::Response

    
```
    ...
    auto res = client.send(HTTP_METHOD::GET, "wirelesstag.net", 80, "");
    // Output response
    std::cout << res->toString() << std::endl;
    ...
    
```

## System requirements
 - rapidjson library
 - fmt library
 - libpq library (optional - using ${ROOT}/PG headers)
 