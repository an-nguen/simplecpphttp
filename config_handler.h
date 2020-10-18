//
// Created by an on 02.07.2020.
//

#ifndef CPPHTTP_CONFIG_HANDLER_H
#define CPPHTTP_CONFIG_HANDLER_H

#include <iostream>
#include <fstream>
#include "nlohmann/json.hpp"

namespace configs {
    using json = nlohmann::json;
    using std::string;

    typedef struct {
        string dbUser;
        string dbPass;
        string dbName;
    } DBConfig;

    int readConfig(const string &filename, json *jsonFile, DBConfig &dbConfig) {
        std::ifstream file(filename);
        if (!file.is_open())
            return -1;

        file >> *jsonFile;
        for (auto& [key, value] : jsonFile->items()) {
            if (key == "dbname") dbConfig.dbName = value;
            if (key == "dbuser") dbConfig.dbUser = value;
            if (key == "dbpass") dbConfig.dbPass = value;
        }
        return 0;
    }

}


#endif //CPPHTTP_CONFIG_HANDLER_H
