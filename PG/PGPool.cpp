//
// Created by an on 22.03.2020.
//

#include <iostream>
#include "PGPool.h"

namespace datasource {
    void PGPool::createPool() {
        std::lock_guard<std::mutex> locker(m_mutex);

        for (auto i = 0; i < m_poolSize; i++) {
            std::shared_ptr<PGConnection> conn(new PGConnection(m_dbHost, m_port, m_dbName, m_dbUser, m_dbPass));
            m_pool.emplace(conn);
        }
        std::cout << "[LOG] - [PGPool] \tThe database connection pool is created." << std::endl;
    }

    std::shared_ptr<PGConnection> PGPool::getConnection() {
        std::unique_lock<std::mutex> lock(m_mutex);

        while (m_pool.empty())
            m_condition.wait(lock);

        auto conn = m_pool.front();
        m_pool.pop();

        return conn;
    }

    void PGPool::freeConnection(const std::shared_ptr<PGConnection>& conn) {
        std::unique_lock<std::mutex> lock(m_mutex);
        m_pool.push(conn);
        lock.unlock();
        m_condition.notify_one();
    }

    PGPool::PGPool(const char *dbHost, unsigned short port, const char *dbName, const char *dbUser,
                   const char *dbPass, int poolSize) : m_dbHost(dbHost), m_port(port), m_dbName(dbName),
                                                       m_dbUser(dbUser), m_dbPass(dbPass),
                                                       m_poolSize(poolSize) {
        createPool();
    }
}