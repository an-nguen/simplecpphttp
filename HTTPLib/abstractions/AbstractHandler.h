//
// Created by an on 10/21/20.
//

#ifndef CPPHTTP_ABSTRACTHANDLER_H
#define CPPHTTP_ABSTRACTHANDLER_H

class AbstractHandler {
public:
    virtual void handle(int epoll_file_descriptor, struct epoll_event &event, int connection_file_descriptor) = 0;
};


#endif //CPPHTTP_ABSTRACTHANDLER_H
