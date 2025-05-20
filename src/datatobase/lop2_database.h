// lop2_database.h
#pragma once

#include "json/json.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sqlite3.h>
#include "lop2_frame.h"


class LOP2Database{
public:  
    explicit LOP2Database(const std::string& dbPath);
    ~LOP2Database();

    void frame_init();

    long frame_insert(const LOP2FrameData& data, size_t len);



private:
    sqlite3* db_ = nullptr;
    std::string dbPath_;

    std::string toHex(const uint8_t* buffer, size_t len);
};
