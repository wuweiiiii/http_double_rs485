#include <iostream>
#include <cstring> 
#include "lop2_frame.h"

// 静态报警映射表初始化
const std::unordered_map<int, std::string> LOP2FrameParser::alarmMap = LOP2FrameParser::initAlarmBitMap();

// 生成报警bit位到文本的映射：Bit编号 = (ByteIndex - 55) * 8 + BitIndex
std::unordered_map <int, std::string> LOP2FrameParser::initAlarmBitMap() {
    std::unordered_map<int, std::string> map;

    // Byte 55
    map[0] = "排气挡板关闭";

    // Byte 56
    map[8] = "机组运行状态";
    map[13] = "机旁/遥控";
    map[14] = "机组备车完毕";
    map[15] = "机组装置罩内1301施救";


    // Byte 57
    map[17] = "机组装置二级报警";
    map[18] = "滑油压力低";
    map[19] = "滑油温度高";
    map[20] = "冷却水压力低";
    map[21] = "发电机绕组温度高";

    // Byte 58
    map[24] = "机组装置一级报警";
    map[25] = "冷却水温度过高";
    map[26] = "滑油压力过低";
    map[27] = "超速停机";
    map[28] = "机组装置罩内灭火紧急停机";
    map[29] = "发电机绕组温度过高跳闸";

    // Byte 59
    map[32] = "燃油泄漏报警";
    map[33] = "罩内空气温度高";
    map[34] = "发电机前轴承温度高";
    map[35] = "发电机后轴承温度高";
    map[36] = "冷却器出口空气度高";
    map[37] = "发电机海水泄漏";
    map[38] = "旋转二极管故障";
    map[39] = "发电机过电压";

    // Byte 60
    map[40] = "机组装置一般故障报警";
    map[41] = "起动空气压力低";
    map[42] = "污油槽液位高";
    map[43] = "A列排温高";
    map[44] = "B列排温高";
    map[45] = "冷却水温度高";
    map[46] = "海水压力低";
    map[47] = "冷却水液位低";

    // Byte 61

    // Byte 62
    map[56] = "燃油压力低";
    map[59] = "罩内风机过载";
    map[60] = "传感器故障";
    map[61] = "回油冷却器海水泄露";


    return map;
}

// 大端字节序解析两个字节为 uint16_t
uint16_t LOP2FrameParser::to_uint16(const uint8_t* ptr) {
    //return ptr[0] | (ptr[1] << 8); 小端
    return (ptr[0] << 8) | ptr[1];
}


std::vector<std::string> LOP2FrameParser::extractActiveAlarms(const uint8_t *buffer)
{
    std::vector<std::string> active;
    for (int byteIdx = 55; byteIdx <= 62; ++byteIdx) {
        uint8_t val = buffer[byteIdx];
        for (int bit = 0; bit < 8; ++bit) {
            if (val & (1 << bit)) {
                int bitIndex = (byteIdx - 55) * 8 + bit;
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

bool LOP2FrameParser::parse(const uint8_t buffer[65], LOP2FrameData& result) {

    std::memcpy(result.ram_frame, buffer, 65);
    result.rpm = to_uint16(&buffer[3]);
    result.runtime = to_uint16(&buffer[5]);
    result.insideairtemp = to_uint16(&buffer[7]) / 10.0f;
    result.oiltemp = to_uint16(&buffer[9]) / 10.0f;
    result.freashwatertemp = to_uint16(&buffer[11]) / 10.0f;
    result.Arowtemp = to_uint16(&buffer[13]) / 10.0f;
    result.Browtemp = to_uint16(&buffer[15]) / 10.0f;
    result.Uphasetemp = to_uint16(&buffer[17]) / 10.0f;
    result.Vphasetemp = to_uint16(&buffer[19]) / 10.0f;
    result.Wphasetemp = to_uint16(&buffer[21]) / 10.0f;
    result.frontbearingtemp = to_uint16(&buffer[23]) / 10.0f;
    result.rearbearingtemp = to_uint16(&buffer[25]) / 10.0f;
    result.inletairtemp = to_uint16(&buffer[27]) / 10.0f;
    result.outletairtemp = to_uint16(&buffer[29]) / 10.0f;
    result.oilpressure = to_uint16(&buffer[31]) / 1000.0f;
    result.airpressure = to_uint16(&buffer[33]) / 1000.0f;
    result.fuelpressure = to_uint16(&buffer[35]) / 1000.0f;

    //检测报警位
    result.activeAlarms = extractActiveAlarms(buffer);
    result.timestamp = std::time(nullptr);
    return true;
}