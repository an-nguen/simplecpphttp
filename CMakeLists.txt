cmake_minimum_required(VERSION 3.16.3)
project(CPPHTTP)
SET(CMAKE_CXX_STANDARD 20)

set(SRC_COMMON
        HTTPLib/HTTPHandler.cpp
        HTTPLib/HTTPUtils.cpp
        HTTPLib/HTTPResponse.h
        HTTPLib/HTTPRequest.h
        HTTPLib/HTTPResource.h
        HTTPLib/HTTP_MACRO.h
        HTTPLib/HTTP_METHOD.h
        HTTPLib/HTTP_STATUS.h
        HTTPLib/HTTP_PROTOCOL.h
        Logger/AbstractLogger.h
        PG/PGConnection.h
        PG/PGPool.h
        PG/DBModel.h
        PG/PGDb.h
        )
set(SRV_SRC
        HTTPLib/Server.h Logger/SimpleLogger.cpp Logger/SimpleLogger.h)
set(CLI_SRC
        HTTPLib/Client.h
        )

find_package (Threads       REQUIRED)
find_package (ZLIB          REQUIRED)
find_package (PostgreSQL    REQUIRED)
find_package (nlohmann_json REQUIRED)
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

file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/config.json DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_SOURCE_DIR}/HTTPLib)
target_include_directories(${PROJECT_NAME} PRIVATE ${PostgreSQL_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PRIVATE ${RapidJSON_INCLUDE_DIRS})
target_link_libraries (${PROJECT_NAME} nlohmann_json::nlohmann_json)
target_link_libraries (${PROJECT_NAME} Threads::Threads)
target_link_libraries (${PROJECT_NAME} ${RapidJSON_LIBRARIES})
target_link_libraries (${PROJECT_NAME} ZLIB::ZLIB)
target_link_libraries (${PROJECT_NAME} ${PostgreSQL_LIBRARIES})
