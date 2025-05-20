#include <iostream>
#include <cstring> 
#include "lop1_frame2.h"

// 静态报警映射表初始化
const std::unordered_map<int, std::string> LOP1Frame2Parser::alarmMap = LOP1Frame2Parser::initAlarmBitMap();

// 生成报警bit位到文本的映射：Bit编号 = (ByteIndex - 21) * 8 + BitIndex
std::unordered_map <int, std::string> LOP1Frame2Parser::initAlarmBitMap() {
    std::unordered_map<int, std::string> map;

    // Byte 21
    map[0] = "1级速度达到";
    map[1] = "2级速度达到";
    map[2] = "3级速度达到";
    map[3] = "4级速度达到";


    // Byte 22
    map[8] = "进气压力传感器电流信号太低";
    map[9] = "进气压力传感器电流信号太高";
    map[10] = "燃油压力传感器电流信号太低";
    map[11] = "燃油压力传感器电流信号太高";
    map[12] = "淡水压力传感器电流信号太低";
    map[13] = "淡水压力传感器电流信号太高";


    // Byte 23
    map[16] = "燃油温度传感器PT1000短路";
    map[17] = "燃油温度传感器PT1000开路";
    map[18] = "进气温度传感器PT1000短路";
    map[19] = "进气温度传感器PT1000开路";


    // Byte 24
    map[24] = "燃油温度高报警";
    map[25] = "进气温度高报警";
    map[27] = "燃油压力低报警";
    map[28] = "淡水压力低报警";

    // Byte 25
    map[34] = "滑油泄露报警";
    map[35] = "起动空气压力低报警";
    map[36] = "自动充油压力低报警";


    return map;
}

// 大端字节序解析两个字节为 uint16_t
uint16_t LOP1Frame2Parser::to_uint16(const uint8_t* ptr) {
    //return ptr[0] | (ptr[1] << 8); 小端
    return (ptr[0] << 8) | ptr[1];
}


std::vector<std::string> LOP1Frame2Parser::extractActiveAlarms(const uint8_t *buffer)
{
    std::vector<std::string> active;
    for (int byteIdx = 21; byteIdx <= 25; ++byteIdx) {
        uint8_t val = buffer[byteIdx];
        for (int bit = 0; bit < 8; ++bit) {
            if (val & (1 << bit)) {
                int bitIndex = (byteIdx - 21) * 8 + bit;
                auto it = alarmMap.find(bitIndex);
                if (it != alarmMap.end()) {
                    active.push_back(it->second);
                } else {
                    active.push_back("未知报警 Bit" + std::to_string(bitIndex));
                }
            }
        }
    }
    return active;
}

bool LOP1Frame2Parser::parse(const uint8_t buffer[32], LOP1Frame2Data& result) {

    std::memcpy(result.ram_frame, buffer, 32);
    result.rpm = to_uint16(&buffer[5]);
    result.oiltemp = to_uint16(&buffer[7]) / 10.0f;
    result.inlettemp = to_uint16(&buffer[9]) / 10.0f;
    result.inletpressure = to_uint16(&buffer[11]) / 100.0f;
    result.oilpressure = to_uint16(&buffer[13]) / 100.0f;
    result.freshwaterpressure = to_uint16(&buffer[15]) / 100.0f;

    //检测报警位
    result.activeAlarms = extractActiveAlarms(buffer);
    result.timestamp = std::time(nullptr);
    return true;
}