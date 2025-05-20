#ifndef _LOP1_FRAME1
#define _LOP1_FRAME1
// lop1_frame_parser.h
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <ctime>
#include <unordered_map>

struct LOP1Frame1Data {
    uint8_t ram_frame[35];
    uint16_t rpm;
    float oilPressure;
    float freshwatertemp;
    uint16_t Arowtemp;
    uint16_t Browtemp;
    float toothoiltemp;
    float toothoilpressure;
    float Seawaterpressure;
    std::vector<std::string> activeAlarms;
    std::time_t timestamp;
};

class LOP1Frame1Parser {
    public:
        bool parse(const uint8_t buffer[35], LOP1Frame1Data& result);

    private:
        uint16_t to_uint16(const uint8_t* ptr);
        std::vector<std::string> extractActiveAlarms(const uint8_t* buffer);
        static std::unordered_map<int, std::string> initAlarmBitMap();
        static const std::unordered_map<int, std::string> alarmMap;
};


#endif