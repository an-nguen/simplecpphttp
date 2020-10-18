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
    class PGPool {
    public:
        PGPool(const char * dbHost, unsigned short port, const char * dbName, const char * dbUser,
               const char * dbPass, int poolSize);
        std::shared_ptr<PGConnection> getConnection();
        void freeConnection(const std::shared_ptr<PGConnection>& conn);

    private:
        void createPool();

        // Database connection config
        std::string m_dbHost;
        int m_port;
        std::string m_dbName;
        std::string m_dbUser;
        std::string m_dbPass;

        int m_poolSize = 16;
        std::mutex m_mutex;
        std::condition_variable m_condition;
        std::queue<std::shared_ptr<PGConnection>> m_pool;

    };

}


#endif //CPPHTTP_PGPOOL_H
