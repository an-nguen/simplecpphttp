//
// Created by an on 22.03.2020.
//
// Take from habr.com/ru/post/322966/ and modified

#ifndef CPPHTTP_PGPOOL_H
#define CPPHTTP_PGPOOL_H


#include <memory>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "PGConnection.h"

namespace datasource {
    template <class L> requires logs::DerivedAbstractLogger<L>
    class PGPool {
    public:
        PGPool(const char *dbHost, unsigned short port,
               const char *dbName, const char *dbUser,
               const char *dbPass, int poolSize,
               L logger) :
               m_dbHost(dbHost), m_port(port),
               m_dbName(dbName), m_dbUser(dbUser),
               m_dbPass(dbPass), m_poolSize(poolSize),
               m_logger(logger) {
            createPool();
        }
        std::shared_ptr<PGConnection<L>> getConnection() {
            std::unique_lock<std::mutex> lock(m_mutex);

            while (m_pool.empty())
                m_condition.wait(lock);

            auto conn = m_pool.front();
            m_pool.pop();

            return conn;
        }
        void freeConnection(const std::shared_ptr<PGConnection<L>>& conn) {
            std::unique_lock<std::mutex> lock(m_mutex);
            m_pool.push(conn);
            lock.unlock();
            m_condition.notify_one();
        }

    private:
        void createPool() {
            std::lock_guard<std::mutex> locker(m_mutex);

            for (auto i = 0; i < m_poolSize; i++) {
                std::shared_ptr<PGConnection<L>> conn(new PGConnection(m_dbHost, m_port, m_dbName, m_dbUser, m_dbPass, m_logger));
                m_pool.emplace(conn);
            }
            m_logger.info("The database connection pool is created.");
        }

        // Database connection config
        std::string m_dbHost;
        int m_port;
        std::string m_dbName;
        std::string m_dbUser;
        std::string m_dbPass;
        L m_logger;

        int m_poolSize = 16;
        std::mutex m_mutex;
        std::condition_variable m_condition;
        std::queue<std::shared_ptr<PGConnection<L>>> m_pool;

    };

}


#endif //CPPHTTP_PGPOOL_H
