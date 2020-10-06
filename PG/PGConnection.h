//
// Created by an on 22.03.2020.
//
// Take from habr.com/ru/post/322966/ and modified

#ifndef CPPHTTP_PGCONNECTION_H
#define CPPHTTP_PGCONNECTION_H

#include <string>
#include <memory>
#include <utility>
#include <libpq-fe.h>
namespace datasource {

    class PGConnection {
    public:
        PGConnection(std::string dbHost, unsigned short port, std::string dbName, std::string dbUser, std::string dbPass) :
                m_dbHost(std::move(dbHost)), m_dbPort(port), m_dbName(std::move(dbName)), m_dbUser(std::move(dbUser)),
                m_dbPass(std::move(dbPass)) {
            establishConnection();
        }

        [[nodiscard]] std::shared_ptr<PGconn> connection() const;
        virtual ~PGConnection() = default;
    private:
        PGConnection() = default;

        void establishConnection();

        std::string m_dbHost;
        unsigned short m_dbPort = 5432;
        std::string m_dbName;
        std::string m_dbUser;
        std::string m_dbPass;

        std::shared_ptr<PGconn> m_connection{};
    };
}

#endif //CPPHTTP_PGCONNECTION_H
