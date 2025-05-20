// lop2_database.cpp
#include "lop2_database.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <json/json.h>

LOP2Database::LOP2Database(const std::string& dbPath) : dbPath_(dbPath) {
    if (sqlite3_open(dbPath_.c_str(), &db_) != SQLITE_OK) {
        std::cerr << "SQLite open failed: " << sqlite3_errmsg(db_) << std::endl;
    }
}

LOP2Database::~LOP2Database() {
    if (db_) sqlite3_close(db_);
}

void LOP2Database::frame_init() {
    const char* createSQL = R"(
        CREATE TABLE IF NOT EXISTS lop2_frame (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            device_id TEXT DEFAULT 'LOP2_frame',        -- 设备标识（如 'LOP2_frame'）
            frame_hex TEXT NOT NULL,                    -- 原始帧的十六进制表示
            rpm INTEGER,                               -- 解析后的转速
            runtime INTEGER,                           -- 运行时间
            insideairtemp REAL,                        -- 内部空气温度 (单位：℃)
            oiltemp REAL,                              -- 机油温度 (单位：℃)
            freashwatertemp REAL,                      -- 淡水温度 (单位：℃)
            Arowtemp REAL,                             -- A 排排温 (单位：℃)
            Browtemp REAL,                             -- B 排排温 (单位：℃)
            Uphasetemp REAL,                           -- U 相温度 (单位：℃)
            Vphasetemp REAL,                           -- V 相温度 (单位：℃)
            Wphasetemp REAL,                           -- W 相温度 (单位：℃)
            frontbearingtemp REAL,                     -- 前轴承温度 (单位：℃)
            rearbearingtemp REAL,                      -- 后轴承温度 (单位：℃)
            inletairtemp REAL,                         -- 进气温度 (单位：℃)
            outletairtemp REAL,                        -- 排气温度 (单位：℃)
            oilpressure REAL,                          -- 机油压力 (单位：MPa)
            airpressure REAL,                          -- 空气压力 (单位：MPa)
            fuelpressure REAL,                         -- 燃油压力 (单位：MPa)
            active_alarms TEXT,                        -- 报警状态，JSON 格式存储
            received_time DATETIME DEFAULT (datetime('now','localtime')) -- 原始数据接收时间
        );
    )";
    char* errMsg = nullptr;
    if (sqlite3_exec(db_, createSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Table creation failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
    
    // 创建 received_time 字段的索引
    const char* createIndexSQL = "CREATE INDEX IF NOT EXISTS idx_received_time ON lop2_frame (received_time);";
    if (sqlite3_exec(db_, createIndexSQL, nullptr, nullptr, &errMsg) != SQLITE_OK) {
        std::cerr << "Index creation failed: " << errMsg << std::endl;
        sqlite3_free(errMsg);
    }
}

std::string LOP2Database::toHex(const uint8_t* buffer, size_t len) {
    std::ostringstream oss;
    for (size_t i = 0; i < len; ++i)
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << (int)buffer[i];
    return oss.str();
}

long LOP2Database::frame_insert(const LOP2FrameData& data, size_t len) {
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
        INSERT INTO lop2_frame 
        (device_id, frame_hex, rpm, runtime, insideairtemp, oiltemp, freashwatertemp, Arowtemp, Browtemp, Uphasetemp, Vphasetemp, Wphasetemp, frontbearingtemp, rearbearingtemp, inletairtemp, outletairtemp, oilpressure, airpressure, fuelpressure, active_alarms)
        VALUES ('LOP2_frame', ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?);
    )", -1, &stmt, nullptr);

    sqlite3_bind_text(stmt, 1, hex.c_str(), -1, SQLITE_TRANSIENT);
    sqlite3_bind_int(stmt, 2, data.rpm);
    sqlite3_bind_int(stmt, 3, data.runtime);
    sqlite3_bind_double(stmt, 4, data.insideairtemp);
    sqlite3_bind_double(stmt, 5, data.oiltemp);
    sqlite3_bind_double(stmt, 6, data.freashwatertemp);
    sqlite3_bind_double(stmt, 7, data.Arowtemp);
    sqlite3_bind_double(stmt, 8, data.Browtemp);
    sqlite3_bind_double(stmt, 9, data.Uphasetemp);
    sqlite3_bind_double(stmt, 10, data.Vphasetemp);
    sqlite3_bind_double(stmt, 11, data.Wphasetemp);
    sqlite3_bind_double(stmt, 12, data.frontbearingtemp);
    sqlite3_bind_double(stmt, 13, data.rearbearingtemp);
    sqlite3_bind_double(stmt, 14, data.inletairtemp);
    sqlite3_bind_double(stmt, 15, data.outletairtemp);
    sqlite3_bind_double(stmt, 16, data.oilpressure);
    sqlite3_bind_double(stmt, 17, data.airpressure);
    sqlite3_bind_double(stmt, 18, data.fuelpressure);
    sqlite3_bind_text(stmt, 19, alarmsStr.c_str(), -1, SQLITE_TRANSIENT);

    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::cerr << "插入 lop2_frame 失败" << std::endl;
        sqlite3_finalize(stmt);
        return -1;
    }

    long rowId = sqlite3_last_insert_rowid(db_);
    sqlite3_finalize(stmt);
    return rowId;
}

