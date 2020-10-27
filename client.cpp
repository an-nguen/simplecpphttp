//
// Created by an on 7/28/20.
//
#include <iostream>
#include "HTTPLib/TCPClient.h"

int main() {
    using namespace cpphttp;
    TCPClient client;
    auto res = client.send(HTTP_METHOD::GET, "wirelesstag.net", 80, "");
    std::cout << res->toString() << std::endl;
    return 0;
}
