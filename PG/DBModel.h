#ifndef CPPHTTP_DBMODEL_H
#define CPPHTTP_DBMODEL_H

#include <map>

class DBModel {
public:
    virtual ~DBModel() = default;
    virtual void init(std::map<std::string, std::string> ) = 0;
    [[nodiscard]] virtual std::string getTableName() const = 0;
};

#endif //CPPHTTP_DBMODEL_H
