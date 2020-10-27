cmake_minimum_required(VERSION 3.16.3)
project(CPPHTTP)
SET(CMAKE_CXX_STANDARD 20)

set(GCC_COVERAGE_COMPILE_FLAGS "-Wall -Wextra -Wconversion")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${GCC_COVERAGE_COMPILE_FLAGS} -DCMAKE_EXPORT_COMPILE_COMMANDS=On" )

set(SRC_COMMON
        HTTPLib/abstractions/AbstractHandler.h
        HTTPLib/domains/HTTPResponse.h
        HTTPLib/domains/HTTPRequest.h
        HTTPLib/implementations/http_handler/HTTPResource.h
        HTTPLib/macro/HTTP_MACRO.h
        HTTPLib/enums/HTTP_METHOD.h
        HTTPLib/enums/HTTP_STATUS.h
        HTTPLib/enums/HTTP_PROTOCOL.h
        HTTPLib/exceptions/TCPClientException.h
        HTTPLib/exceptions/TCPServerException.h
        HTTPLib/exceptions/HTTPHandlerException.h
        PG/PGConnection.h
        PG/PGPool.h
        PG/DBModel.h
        PG/PGDb.h
        Logger/AbstractLogger.h
        Logger/SimpleLogger.cpp
        Logger/SimpleLogger.h
        )
set(SRV_SRC
        HTTPLib/TCPServer.h)
set(CLI_SRC
        HTTPLib/TCPClient.h
        )

find_package (Threads       REQUIRED)
find_package (PostgreSQL    REQUIRED)
find_package (RapidJSON     REQUIRED)

add_executable(CPPHTTP server.cpp ${SRV_SRC} ${SRC_COMMON})
add_executable(CPPHTTP_Client client.cpp ${CLI_SRC} ${SRC_COMMON})

ADD_CUSTOM_TARGET(
        copy_config_file
        ${CMAKE_COMMAND}
        -D SOURCE_DIR=${CMAKE_CURRENT_SOURCE_DIR}
        -D DESTINATION_DIR=${CMAKE_CURRENT_BINARY_DIR}
        -P config.json
        COMMENT "Copying Files for target: CPPHTTP" VERBATIM
)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/HTTPLib)
target_include_directories(${PROJECT_NAME} PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${RapidJSON_INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} Threads::Threads)
target_link_libraries (${PROJECT_NAME} ${RapidJSON_LIBRARIES})
target_link_libraries (${PROJECT_NAME} ${PostgreSQL_LIBRARIES})
