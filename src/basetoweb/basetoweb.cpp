
#include "basetoweb.h"
#include <sqlite3.h>
#include <iostream>
#include <sstream>
#include <json/json.h>

BaseToWeb::BaseToWeb(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
        db = nullptr;
    }
}

BaseToWeb::~BaseToWeb() {
    if (db) {
        sqlite3_close(db);
    }
}

Json::Value BaseToWeb::handleRealtime(const Json::Value& request) {
    Json::Value modifiedRequest = request;
    modifiedRequest["sort"]["field"] = "received_time";
    modifiedRequest["sort"]["order"] = "DESC";
    modifiedRequest["pagination"]["offset"] = 0;
    modifiedRequest["pagination"]["limit"] = 1;
    return handleQuery(modifiedRequest);
}

Json::Value BaseToWeb::handleDelete(const Json::Value& request) {
    Json::Value response;
    if (!db) {
        response["status"] = "error";
        response["message"] = "Database not opened.";
        return response;
    }

    std::string table = request["table"].asString();
    std::string start, end;
    if (request.isMember("filter") && request["filter"].isMember("time_range")) {
        const auto& range = request["filter"]["time_range"];
        if (range.isMember("start")) start = range["start"].asString();
        if (range.isMember("end")) end = range["end"].asString();
    }

    std::string whereClause;
    if (!start.empty() && !end.empty()) {
        whereClause = "received_time BETWEEN '" + start + "' AND '" + end + "'";
    } else if (!start.empty()) {
        whereClause = "received_time >= '" + start + "'";
    } else if (!end.empty()) {
        whereClause = "received_time <= '" + end + "'";
    } else {
        response["status"] = "error";
        response["message"] = "Missing time range";
        return response;
    }

    std::string countSql = "SELECT COUNT(*) FROM " + table + " WHERE " + whereClause;
    int rowsBefore = 0;
    char* errMsg = nullptr;
    sqlite3_exec(db, countSql.c_str(), [](void* data, int argc, char** argv, char**) {
        if (argc > 0 && argv[0]) {
            int* count = static_cast<int*>(data);
            *count = std::stoi(argv[0]);
        }
        return 0;
    }, &rowsBefore, &errMsg);

    std::string sql = "DELETE FROM " + table + " WHERE " + whereClause;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        response["status"] = "error";
        response["message"] = errMsg;
        sqlite3_free(errMsg);
    } else {
        response["status"] = "success";
        response["deleted"] = rowsBefore;
        response["message"] = "Data records deleted successfully.";
    }

    return response;
}

Json::Value BaseToWeb::handleQuery(const Json::Value& request) {
    Json::Value response;

    sqlite3* readDb = nullptr;
    if (sqlite3_open_v2("/userdata/sqlite/lop1.db", &readDb, SQLITE_OPEN_READONLY, nullptr) != SQLITE_OK) {
        response["status"] = "error";
        response["message"] = "Read DB open failed.";
        return response;
    }

    std::string sql = generateQuerySql(request);
    std::vector<std::vector<std::string>> results;

    char* errMsg = nullptr;
    sqlite3_exec(readDb, sql.c_str(), [](void* data, int argc, char** argv, char**) {
        auto* results = static_cast<std::vector<std::vector<std::string>>*>(data);
        std::vector<std::string> row;
        for (int i = 0; i < argc; ++i) {
            row.emplace_back(argv[i] ? argv[i] : "NULL");
        }
        results->push_back(row);
        return 0;
    }, &results, &errMsg);

    sqlite3_close(readDb);

    if (!errMsg && !results.empty()) {
        response["status"] = "success";
        response["message"] = "Query processed successfully";
        Json::Value data;
        for (const auto& row : results) {
            Json::Value rowData;
            for (const auto& col : row) {
                rowData.append(col);
            }
            data.append(rowData);
        }
        response["data"] = data;

        if (request.isMember("fields") && request["fields"].isArray()) {
            const Json::Value& fields = request["fields"];
            Json::Value columns(Json::arrayValue);
            for (Json::ArrayIndex i = 0; i < fields.size(); ++i) {
                columns.append(fields[i].asString());
            }
            response["columns"] = columns;
        }
    } else {
        response["status"] = "error";
        response["message"] = errMsg ? errMsg : "No results found.";
        if (errMsg) sqlite3_free(errMsg);
    }

    return response;
}

std::vector<std::vector<std::string>> BaseToWeb::executeQuery(const std::string& sql) {
    std::vector<std::vector<std::string>> results;
    char* errMsg = nullptr;

    if (sqlite3_exec(db, sql.c_str(), [](void* data, int argc, char** argv, char**) {
        std::vector<std::vector<std::string>>* results = static_cast<std::vector<std::vector<std::string>>*>(data);
        std::vector<std::string> row;
        for (int i = 0; i < argc; ++i) {
            row.emplace_back(argv[i] ? argv[i] : "NULL");
        }
        results->push_back(row);
        return 0;
    }, &results, &errMsg) != SQLITE_OK) {
        std::cerr << "SQL error: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }

    return results;
}

std::string BaseToWeb::generateQuerySql(const Json::Value& request) {
    std::string table_name = request["table"].asString();
    const Json::Value& fields = request["fields"];
    std::string sql;

    if (!fields.isArray() || fields.empty()) {
        return "ERROR: Fields array missing or invalid.";
    }

    bool useAllFields = (fields.size() == 1 && fields[0].asString() == "*");

    if (useAllFields) {
        sql = "SELECT * FROM " + table_name;
    } else {
        sql = "SELECT ";
        for (Json::ArrayIndex i = 0; i < fields.size(); ++i) {
            if (!fields[i].isString()) {
                return "ERROR: Field name must be string.";
            }
            sql += fields[i].asString();
            if (i != fields.size() - 1) sql += ", ";
        }
        sql += " FROM " + table_name;
    }

    std::vector<std::string> where_clauses;

    if (request.isMember("filter") && request["filter"].isObject()) {
        const Json::Value& filter = request["filter"];

        if (filter.isMember("time_range") && filter["time_range"].isObject()) {
            const Json::Value& timeRange = filter["time_range"];
            std::string start, end;

            if (timeRange.isMember("start")) start = timeRange["start"].asString();
            if (timeRange.isMember("end")) end = timeRange["end"].asString();

            if (!start.empty() && !end.empty()) {
                where_clauses.push_back("received_time BETWEEN '" + start + "' AND '" + end + "'");
            } else if (!start.empty()) {
                where_clauses.push_back("received_time >= '" + start + "'");
            } else if (!end.empty()) {
                where_clauses.push_back("received_time <= '" + end + "'");
            }
        }

        if (filter.isMember("conditions") && filter["conditions"].isArray()) {
            for (const auto& cond : filter["conditions"]) {
                std::string field = cond["field"].asString();
                std::string op = cond["op"].asString();
                std::string val = cond["value"].asString();
                where_clauses.push_back(field + " " + op + " '" + val + "'");
            }
        }
    }

    if (!where_clauses.empty()) {
        sql += " WHERE ";
        for (size_t i = 0; i < where_clauses.size(); ++i) {
            sql += where_clauses[i];
            if (i != where_clauses.size() - 1) sql += " AND ";
        }
    }

    if (request.isMember("sort") && request["sort"].isObject()) {
        std::string column = request["sort"]["field"].asString();
        std::string order = request["sort"]["order"].asString();
        sql += " ORDER BY " + column + " " + order;
    }

    if (request.isMember("pagination") && request["pagination"].isObject()) {
        int offset = request["pagination"].get("offset", 0).asInt();
        int limit = request["pagination"].get("limit", 100).asInt();
        sql += " LIMIT " + std::to_string(limit) + " OFFSET " + std::to_string(offset);
    }

    sql += ";";
    return sql;
}
