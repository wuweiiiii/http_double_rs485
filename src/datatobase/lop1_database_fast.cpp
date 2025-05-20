// lop1_database.cpp
#include "lop1_database_fast.h"

LOP1Database::LOP1Database(const std::string& dbPath) : dbPath_(dbPath) {
    if (sqlite3_open(dbPath_.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "SQLite open failed: " << sqlite3_errmsg(db_) << std::endl;
    }else{
        // 启用 WAL 模式
        sqlite3_exec(db_, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
        sqlite3_busy_timeout(db_, 1000); //1秒超时
    }

}

LOP1Database::~LOP1Database() {
    if (db_) sqlite3_close(db_);
}

void LOP1Database::frame1_init() {
    const char* createSQL = R"(
        CREATE TABLE IF NOT EXISTS lop1_frame1 (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_id TEXT DEFAULT 'LOP1_frame1',        -- 设备标识（如 'LOP1_frame1'）
            frame_hex TEXT NOT NULL,                    -- 原始帧的十六进制表示
            rpm1 INTEGER,                               -- 解析后的转速
            oil_pressure REAL,                          -- 滑油压力 (单位：bar)
            freshwater_temp REAL,                      -- 淡水温度 (单位：℃)
            a排排温 INTEGER,                            -- A 排排温 (单位：℃)
            b排排温 INTEGER,                            -- B 排排温 (单位：℃)
            齿油温 REAL,                              -- 齿油温度 (单位：℃)
            齿油压 REAL,                              -- 齿油压力 (单位：bar)
            海水压 REAL,                              -- 海水压力 (单位：bar)
            active_alarms TEXT,                        -- 报警状态，JSON 格式存储
            received_time DATETIME DEFAULT (datetime('now','localtime')) -- 原始数据接收时间
        );
    )";
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, createSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Table creation failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    sqlite3_exec(db_, "CREATE INDEX IF NOT EXISTS idx_frame1_time ON lop1_frame1(received_time DESC);", nullptr, nullptr, nullptr);
}

void LOP1Database::frame2_init() {
    const char* createSQL = R"(
        CREATE TABLE IF NOT EXISTS lop1_frame2 (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_id TEXT DEFAULT 'LOP1_frame2',        -- 设备标识（如 'LOP1_frame2'）
            frame_hex TEXT NOT NULL,                    -- 原始帧的十六进制表示
            rpm2 INTEGER,                               -- 解析后的转速
            oil_temp REAL,                         -- 滑油温度 (单位：bar)
            inlet_temp REAL,                        -- 进气温度 (单位：℃)
            inlet_pressure REAL,                      -- 进气压力 (单位：℃)
            燃油压 REAL,                               -- 燃油压力 (单位：bar)
            淡水压 REAL,                               -- 淡水压力 (单位：bar)
            active_alarms TEXT,                        -- 报警状态，JSON 格式存储
            received_time DATETIME DEFAULT (datetime('now','localtime')) -- 原始数据接收时间
        );
    )";
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, createSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Table creation failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    sqlite3_exec(db_, "CREATE INDEX IF NOT EXISTS idx_frame2_time ON lop1_frame2(received_time DESC);", nullptr, nullptr, nullptr);
}

std::string LOP1Database::toHex(const uint8_t* buffer, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i)
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)buffer[i];
    return oss.str();
}

long LOP1Database::frame1_insert(const LOP1Frame1Data& data, size_t len) {
    // 将原始数据帧转换为十六进制字符串
    std::string hex = toHex(data.ram_frame, len);

    // 处理报警状态为 JSON 字符串
    Json::Value alarmsJson(Json::arrayValue);
    for (const auto& alarm : data.activeAlarms) {
        alarmsJson.append(alarm);
    }
    Json::StreamWriterBuilder writerBuilder;
    writerBuilder.settings_["emitUTF8"] = true;
    std::string alarmsStr = Json::writeString(writerBuilder, alarmsJson);

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, R"(
        INSERT INTO lop1_frame1 
        (device_id, frame_hex, rpm1, oil_pressure, freshwater_temp, a排排温, b排排温, 齿油温, 齿油压, 海水压, active_alarms)
        VALUES ('LOP1_frame1', ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )", -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, hex.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, data.rpm);
    sqlite3_bind_double(stmt, 3, data.oilPressure);
    sqlite3_bind_double(stmt, 4, data.freshwatertemp);
    sqlite3_bind_int(stmt, 5, data.Arowtemp);
    sqlite3_bind_int(stmt, 6, data.Browtemp);
    sqlite3_bind_double(stmt, 7, data.toothoiltemp);
    sqlite3_bind_double(stmt, 8, data.toothoilpressure);
    sqlite3_bind_double(stmt, 9, data.Seawaterpressure);
    sqlite3_bind_text(stmt, 10, alarmsStr.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        if (rc == SQLITE_BUSY) {
            std::cerr << "插入失败：数据库正被锁定，SQLITE_BUSY" << std::endl;
        } else {
            std::cerr << "插入 lop1_frame1 失败, 错误码: " << rc << std::endl;
        }
        sqlite3_finalize(stmt);
        return -1;
    }

    long rowId = sqlite3_last_insert_rowid(db_);
    sqlite3_finalize(stmt);
    return rowId;
}


long LOP1Database::frame2_insert(const LOP1Frame2Data& data, size_t len) {
    // 将原始数据帧转换为十六进制字符串
    std::string hex = toHex(data.ram_frame, len);

    // 处理报警状态为 JSON 字符串
    Json::Value alarmsJson(Json::arrayValue);
    for (const auto& alarm : data.activeAlarms) {
        alarmsJson.append(alarm);
    }
    Json::StreamWriterBuilder writerBuilder;
    writerBuilder.settings_["emitUTF8"] = true;
    std::string alarmsStr = Json::writeString(writerBuilder, alarmsJson);

    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(db_, R"(
        INSERT INTO lop1_frame2 
        (device_id, frame_hex, rpm2, oil_temp, inlet_temp, inlet_pressure, 燃油压, 淡水压, active_alarms)
        VALUES ('LOP1_frame2', ?, ?, ?, ?, ?, ?, ?, ?);
    )", -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, hex.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, data.rpm);
    sqlite3_bind_double(stmt, 3, data.oiltemp);
    sqlite3_bind_double(stmt, 4, data.inlettemp);
    sqlite3_bind_int(stmt, 5, data.inletpressure);
    sqlite3_bind_int(stmt, 6, data.oilpressure);
    sqlite3_bind_double(stmt, 7, data.freshwaterpressure);
    sqlite3_bind_text(stmt, 8, alarmsStr.c_str(), -1, SQLITE_TRANSIENT);

    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE) {
        if (rc == SQLITE_BUSY) {
            std::cerr << "插入失败：数据库正被锁定，SQLITE_BUSY" << std::endl;
        } else {
            std::cerr << "插入 lop1_frame1 失败, 错误码: " << rc << std::endl;
        }
        sqlite3_finalize(stmt);
        return -1;
    }

    long rowId = sqlite3_last_insert_rowid(db_);
    sqlite3_finalize(stmt);
    return rowId;
}

void LOP1Database::beginTransaction() {
    sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
}

void LOP1Database::commitTransaction() {
    sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, nullptr);
}
