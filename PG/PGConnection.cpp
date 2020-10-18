//
// Created by an on 22.03.2020.
//

#include <iostream>
#include "PGConnection.h"
namespace datasource {
    std::shared_ptr<PGconn> PGConnection::connection() const {
        return m_connection;
    }

    void PGConnection::establishConnection() {
        // reset a shared pointer, hand it a fresh instance of pq_conn
        // the old instance will be destroyed after call reset if it is defined
        m_connection.reset(PQsetdbLogin(m_dbHost.c_str(), std::to_string(m_dbPort).c_str(), nullptr, nullptr,
                                        m_dbName.c_str(), m_dbUser.c_str(), m_dbPass.c_str()), &PQfinish);
        // check connection
        if (PQstatus(m_connection.get()) != CONNECTION_OK && PQsetnonblocking(m_connection.get(), 1) != 0)
            throw std::runtime_error(PQerrorMessage(m_connection.get()));
        std::cout << "[LOG] - [PGConnection]\tConnection is established." << std::endl;
    }
}