//
// Created by an on 09.07.2020.
//

#ifndef CPPHTTP_PGDB_H
#define CPPHTTP_PGDB_H

#include <string>
#include <cstring>
#include <algorithm>
#include <utility>
#include <malloc.h>
#include "PGPool.h"
#include "DBModel.h"


#define Assert(condition, throw_str) if (condition) throw std::runtime_error(throw_str)

static const std::array M_NOT_PERMITTED_COMMANDS = {
        "select ", "update ", "into ", "from ", "where ", "limit ",
        "insert ", "asc ", "desc ", "explain ", "delete ", "create ",
        "drop ", "values ", " table ", " database ", " function ", " with ",
        " role ", " declare "
};

namespace datasource {
    using std::string;
    using std::vector;
    using std::shared_ptr;
    using std::runtime_error;

    template <class T>
    concept DerivedDBModel = std::is_base_of<DBModel, T>::value;

    template <class L> requires logs::DerivedAbstractLogger<L>
    class PGDb {
    public:
        PGDb(shared_ptr<PGPool<L>> pool, string conditions) : m_pool(std::move(pool)), m_where_conditions(std::move(conditions)) {
        }
        PGDb(const char * dbHost, int port, const char * dbName, const char * dbUser,
             const char * dbPass, int poolSize, L logger) {
            this->m_pool = std::make_shared<PGPool<L>>(dbHost, port, dbName, dbUser, dbPass, poolSize, logger);
        }

        static bool isSQLInjection(const string &param) {
            string val(param);
            std::transform(val.begin(), val.end(), val.begin(), ::tolower);
            if (std::ranges::any_of(M_NOT_PERMITTED_COMMANDS.begin(), M_NOT_PERMITTED_COMMANDS.end(), [=](auto i) {
                return val.find(std::string(i)) != std::string::npos;
            })) {
                return true;
            }

            return false;
        }

        template<class T> requires DerivedDBModel<T>
        PGDb * Find(const string &table, vector<T> &result) {
            if (isSQLInjection(table)) {
                throw runtime_error("sql injection detected!");
            }

            int sendResult; T tmpObj{};
            auto PGconn = this->m_pool->getConnection();
            auto conn = PGconn->connection().get();
            std::string preparedQuery = "select * from " + tmpObj.getTableName();
            if (!this->m_where_conditions.empty()) {
                preparedQuery += " where " + this->m_where_conditions;
            }
            if (!this->m_order_conditions.empty()) {
                preparedQuery += " order by " + this->m_order_conditions;
            }
            if (this->m_limit != 0) {
                preparedQuery += " limit " + std::to_string(this->m_limit);
            }

            if (!this->m_query_param_values.empty()) {
                // Allocate array of strings and copy from vector
                char **str_val;
                str_val = (char **)malloc(m_query_param_values.size() * sizeof(char* ));
                for (size_t i = 0; i < m_query_param_values.size(); i++) {
                    str_val[i] = (char *)malloc((m_query_param_values.at(i).length() + 1) * sizeof(char *));
                    strcpy(str_val[i], m_query_param_values.at(i).c_str());
                }
                sendResult = PQsendQueryParams(conn, preparedQuery.c_str(),
                                               static_cast<int>(this->m_query_param_values.size()),
                                               nullptr, str_val,
                                               nullptr, nullptr, 0);
                // Free allocated array
                for (size_t i = 0; i < m_query_param_values.size(); i++)
                    free(str_val[i]);

                free(str_val);
                Assert(sendResult != 1, "failed to exec statement");
            } else {
                sendResult = PQsendQuery(conn, preparedQuery.c_str());
                Assert(sendResult != 1, "failed to exec statement");
            }
            PGresult *queryRes;
            while ( queryRes = PQgetResult(conn) ) {
                auto nCols = PQnfields(queryRes);
                auto mRows = PQntuples(queryRes);

                if (PQresultStatus(queryRes) == PGRES_TUPLES_OK && PQntuples(queryRes)) {
                    for (int r = 0; r < mRows; r++) {
                        std::map<std::string,std::string> record;
                        for (int n = 0; n < nCols; n++) {
                            string key (PQfname(queryRes, n));
                            string val (PQgetvalue(queryRes, r, n));
                            record.insert(std::pair<std::string, std::string>(key, val));
                        }
                        T obj{};
                        obj.init(record);
                        result.push_back(obj);
                    }
                }

                Assert(PQresultStatus(queryRes) == PGRES_FATAL_ERROR, string(PQresultErrorMessage(queryRes)));
                PQclear(queryRes);
            }

            PQflush(conn);
            this->m_pool->freeConnection(PGconn);
            this->clear();

            return this;
        }

        template<class T> requires DerivedDBModel<T>
        PGDb * first(const string &table, vector<T> &result) {
            this->m_limit = 1;
            return this->Find(table, result);
        }

        PGDb * limit(long long countRows) {
            this->m_limit = countRows;
            return this;
        }

        void clear() {
            this->m_limit = 0;
            m_where_conditions.clear();
            m_query_param_values.clear();
            m_order_conditions.clear();
        }

        template<class ...Params>
        PGDb * Where(const char * format, Params... args) {
            constexpr size_t len = sizeof...(Params);
            string fmt(format);
            fmt.erase(std::remove_if(fmt.begin(), fmt.end(), [](int ch) {
                return std::isspace(ch);
            }), fmt.end());
            size_t pos = fmt.find('?');
            size_t cnt = 0;
            while (pos != string::npos) {
                cnt++;
                string val ("$" + std::to_string(cnt));
                fmt.replace(pos, val.length(), val);
                pos = fmt.find('?', pos + 1);
            }

            if (len != cnt) {
                throw runtime_error(std::to_string(len) + " != " + std::to_string(cnt));
            }
            m_query_param_values.clear();

            m_where_conditions.assign(fmt);
            m_query_param_values.push_back(args...);

            return this;
        }

        PGDb * OrderBy(const char *fmt) {
            this->m_order_conditions.assign(fmt);
            return this;
        }

        PGDb * Limit(unsigned int lim) {
            this->m_limit = lim;
            return this;
        }

    private:
        shared_ptr<PGPool<L>> m_pool;
        string m_where_conditions{};
        string m_order_conditions{};

        unsigned int m_limit = 0;
        vector<string> m_query_param_values{};
    };
}



#endif //CPPHTTP_PGDB_H
