// lop1_database.h
#pragma once

#include "json/json.h"
#include <sstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <sqlite3.h>
#include "lop1_frame1.h"
#include "lop1_frame2.h"

class LOP1Database{
public:
    explicit LOP1Database(const std::string& dbPath);
    ~LOP1Database();

    void frame1_init();
    void frame2_init();

    long frame1_insert(const LOP1Frame1Data& data, size_t len);
    long frame2_insert(const LOP1Frame2Data& data, size_t len);

    void beginTransaction();
    void commitTransaction();


private:
    sqlite3* db_ = nullptr;
    std::string dbPath_;

    std::string toHex(const uint8_t* buffer, size_t len);
};
