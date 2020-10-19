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
#include "../Logger/AbstractLogger.h"

namespace datasource {

    template <class L> requires logs::DerivedAbstractLogger<L>
    class PGConnection {
    public:
        PGConnection(std::string dbHost, unsigned short port, std::string dbName, std::string dbUser, std::string dbPass, L logger) :
                m_dbHost(std::move(dbHost)), m_dbPort(port), m_dbName(std::move(dbName)), m_dbUser(std::move(dbUser)),
                m_dbPass(std::move(dbPass)) {
            establishConnection();
        }

        [[nodiscard]] std::shared_ptr<PGconn> connection() const {
            return m_connection;
        }
        virtual ~PGConnection() = default;
    private:
        PGConnection() = default;

        void establishConnection()  {
            // reset a shared pointer, hand it a fresh instance of pq_conn
            // the old instance will be destroyed after call reset if it is defined
            m_connection.reset(PQsetdbLogin(m_dbHost.c_str(), std::to_string(m_dbPort).c_str(), nullptr, nullptr,
                                            m_dbName.c_str(), m_dbUser.c_str(), m_dbPass.c_str()), &PQfinish);
            // check connection
            if (PQstatus(m_connection.get()) != CONNECTION_OK && PQsetnonblocking(m_connection.get(), 1) != 0)
                throw std::runtime_error(PQerrorMessage(m_connection.get()));
            m_logger.info("Connection is established.");
        }

        std::string m_dbHost;
        unsigned short m_dbPort = 5432;
        std::string m_dbName;
        std::string m_dbUser;
        std::string m_dbPass;
        L m_logger;

        std::shared_ptr<PGconn> m_connection{};
    };
}

#endif //CPPHTTP_PGCONNECTION_H
