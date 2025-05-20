#ifndef _LOP2_FRAME
#define _LOP2_FRAME
// lop2_frame.h
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <ctime>
#include <unordered_map>

struct LOP2FrameData {
    uint8_t ram_frame[65];
    uint16_t rpm;
    uint16_t runtime;
    float insideairtemp;
    float oiltemp;
    float freashwatertemp;
    float Arowtemp;
    float Browtemp;
    float Uphasetemp;
    float Vphasetemp;
    float Wphasetemp;
    float frontbearingtemp;
    float rearbearingtemp;
    float inletairtemp;
    float outletairtemp;
    float oilpressure;
    float airpressure;
    float fuelpressure;
    std::vector<std::string> activeAlarms;
    std::time_t timestamp;
};

class LOP2FrameParser {
    public:
        bool parse(const uint8_t buffer[65], LOP2FrameData& result);

    private:
        uint16_t to_uint16(const uint8_t* ptr);
        std::vector<std::string> extractActiveAlarms(const uint8_t* buffer);
        static std::unordered_map<int, std::string> initAlarmBitMap();
        static const std::unordered_map<int, std::string> alarmMap;
};


#endif