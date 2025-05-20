#ifndef BASETOWEB_H
#define BASETOWEB_H

#include <json/json.h>
#include <string>
#include <vector>

class sqlite3;

class BaseToWeb {
public:
    BaseToWeb(const std::string& dbPath);
    ~BaseToWeb();

    Json::Value handleQuery(const Json::Value& request);
    Json::Value handleRealtime(const Json::Value& request);
    Json::Value handleDelete(const Json::Value& request);

private:
    sqlite3* db;
    std::vector<std::vector<std::string>> executeQuery(const std::string& sql);
    std::string generateQuerySql(const Json::Value& request);
};

#endif // BASETOWEB_H