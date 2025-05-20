#include <iostream>
#include <cstring> 
#include "lop1_frame1.h"

// 静态报警映射表初始化
const std::unordered_map<int, std::string> LOP1Frame1Parser::alarmMap = LOP1Frame1Parser::initAlarmBitMap();

// 生成报警bit位到文本的映射：Bit编号 = (ByteIndex - 21) * 8 + BitIndex
std::unordered_map <int, std::string> LOP1Frame1Parser::initAlarmBitMap() {
    std::unordered_map<int, std::string> map;

    // Byte 21
    map[1] = "ALSY 机旁紧急停机键断线";
    map[2] = "ALSY 遥控紧急停机键断线";
    map[4] = "ALSY 淡水温度传感器开路";
    map[5] = "ALSY 淡水温度传感器短路";
    map[6] = "ALSY 齿油温度传感器开路";
    map[7] = "ALSY 齿油温度传感器短路";

    // Byte 22
    map[8] = "ALSY 海水压力传感器电流信号太低";
    map[9] = "ALSY 海水压力传感器电流信号太高";
    map[10] = "ALSY 齿油压力传感器电流信号太高";
    map[11] = "ALSY 齿油压力传感器电流信号太低";
    map[12] = "ALSY 滑油压力传感器电流信号太高";
    map[13] = "ALSY 滑油压力传感器电流信号太低";
    map[14] = "ALSY 越控键断线";

    // Byte 23
    map[17] = "SISY 遥控紧急停机键断线";
    map[18] = "SISY 机旁紧急停机键断线";
    map[19] = "SISY 淡水温度传感器短路";
    map[20] = "SISY 淡水温度传感器开路";
    map[22] = "SISY 滑油压力传感器电流信号太高";
    map[23] = "SISY 滑油压力传感器电流信号太低";

    // Byte 24
    map[24] = "SISY 越控键断线";
    map[25] = "SISY 进气挡板继电器断线";
    map[26] = "ALSY+SISY 转速传感器故障";

    // Byte 25
    map[32] = "淡水泄漏报警";
    map[33] = "燃油泄漏报警";
    map[34] = "进气挡板关闭报警";
    map[35] = "机旁报警确认";
    map[36] = "遥控起动";
    map[37] = "遥控停机";
    map[38] = "紧急停机";
    map[39] = "越控";

    // Byte 26
    map[41] = "报警复位";
    map[42] = "备车";
    map[43] = "遥控起动释放";
    map[44] = "“机旁”控制";
    map[45] = "停机（转速低于 50)";
    map[46] = "起动（转速低于 300)";

    // Byte 27
    map[48] = "转速大于 300";
    map[49] = "超速紧急停机";
    map[50] = "淡水温度高报警";
    map[51] = "淡水温度太高停机";
    map[52] = "滑油压力低报警";
    map[53] = "滑油压力太低停机";
    map[54] = "进气挡板关闭";

    // Byte 28
    map[56] = "海水压力低报警";
    map[57] = "齿油压力低报警";
    map[58] = "齿油温度高报警";
    map[59] = "A 排排温高报警";
    map[60] = "B 排排温高报警";
    map[61] = "齿油压力太低停机";
    map[62] = "油中进水报警";
    map[63] = "起动失败";

    // Byte 29
    map[64] = "ALSY 传感器故障报警";
    map[66] = "ALSY 紧急停机测试";
    map[67] = "SISY 紧急停机测试";
    map[68] = "SISY 传感器故障报警";
    map[69] = "SISY 转速传感器故障报警";
    map[70] = "ALSY 转速传感器故障报警";

    return map;
}

// 大端字节序解析两个字节为 uint16_t
uint16_t LOP1Frame1Parser::to_uint16(const uint8_t* ptr) {
    //return ptr[0] | (ptr[1] << 8); 小端
    return (ptr[0] << 8) | ptr[1];
}


std::vector<std::string> LOP1Frame1Parser::extractActiveAlarms(const uint8_t *buffer)
{
    std::vector<std::string> active;
    for (int byteIdx = 21; byteIdx <= 29; ++byteIdx) {
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

bool LOP1Frame1Parser::parse(const uint8_t buffer[35], LOP1Frame1Data& result) {

    std::memcpy(result.ram_frame, buffer, 35);
    result.rpm = to_uint16(&buffer[5]);
    result.oilPressure = to_uint16(&buffer[7]) / 100.0f;
    result.freshwatertemp = to_uint16(&buffer[9]) / 10.0f;
    result.Arowtemp = to_uint16(&buffer[11]);
    result.Browtemp = to_uint16(&buffer[13]);
    result.toothoiltemp = to_uint16(&buffer[15]) / 10.0f;
    result.toothoilpressure = to_uint16(&buffer[17]) / 100.0f;
    result.Seawaterpressure = to_uint16(&buffer[19]) / 100.0f;

    //检测报警位
    result.activeAlarms = extractActiveAlarms(buffer);
    result.timestamp = std::time(nullptr);
    return true;
}