#ifndef _LOP1_FRAME2
#define _LOP1_FRAME2
// lop1_frame_parser.h
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <ctime>
#include <unordered_map>

struct LOP1Frame2Data {
    uint8_t ram_frame[32];
    uint16_t rpm;
    float oiltemp;
    float inlettemp;
    float inletpressure;
    float oilpressure;
    float freshwaterpressure;
    std::vector<std::string> activeAlarms;
    std::time_t timestamp;
};

class LOP1Frame2Parser {
    public:
        bool parse(const uint8_t buffer[32], LOP1Frame2Data& result);

    private:
        uint16_t to_uint16(const uint8_t* ptr);
        std::vector<std::string> extractActiveAlarms(const uint8_t* buffer);
        static std::unordered_map<int, std::string> initAlarmBitMap();
        static const std::unordered_map<int, std::string> alarmMap;
};


#endif