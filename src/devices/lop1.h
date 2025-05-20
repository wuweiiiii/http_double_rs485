// lop1.h
#ifndef _LOP1_H
#define _LOP1_H

#include "linux_uart.h"
#include <string>
using namespace std;

class Lop1 {
public:
    Lop1(const string &deviceName, int baudRate = 9600);
    ~Lop1();

    static constexpr int TEMP_CAP    = 128;
    static constexpr int MIN_FRAME   = 7;
    static constexpr int FRAME1_LEN  = 35;  // 长度字段 0x0023
    static constexpr int FRAME2_LEN  = 32;  // 长度字段 0x0020

    bool initialize();

    // 第一种帧
    bool receiveData1(uint8_t* buffer);
    bool validateFrame1(const uint8_t* frameBuf) const;
    void printReceivedData1(const uint8_t* buffer);

    // 第二种帧
    bool receiveData2(uint8_t* buffer);
    bool validateFrame2(const uint8_t* frameBuf) const;
    void printReceivedData2(const uint8_t* buffer);

private:
    LinuxUart *uart;
    string deviceName;
    int baudRate;

    uint8_t tempBuffer[TEMP_CAP];
    int     tempBufferLen = 0;
};

#endif
