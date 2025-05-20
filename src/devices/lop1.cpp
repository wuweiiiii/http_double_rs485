// lop1.cpp
#include "lop1.h"
#include <cstring>
#include <iostream>
using namespace std;

Lop1::Lop1(const string& deviceName,int baudRate)
  : deviceName(deviceName), baudRate(baudRate)
{
    uart = new LinuxUart(deviceName, baudRate);
}

Lop1::~Lop1(){
    delete uart;
}

bool Lop1::initialize(){
    return uart->defaultInit(baudRate);
}

// 第一种帧接收
bool Lop1::receiveData1(uint8_t* frameBuf)
{
    // 1) 读串口
    int r = uart->readData(tempBuffer + tempBufferLen,
                           TEMP_CAP - tempBufferLen);
    if (r <= 0) return false;
    tempBufferLen += r;

    // 2) 搜帧头 FA F5
    for (int i = 0; i <= tempBufferLen - MIN_FRAME; ++i) {
        if (tempBuffer[i] == 0xFA && tempBuffer[i+1] == 0xF5) {
            // 3) 取长度
            uint16_t frameLen = (uint16_t(tempBuffer[i+2]) << 8)
                              | uint16_t(tempBuffer[i+3]);
            if (frameLen != FRAME1_LEN) continue;
            //if (frameLen < MIN_FRAME || frameLen > TEMP_CAP) continue;
            
            // 4) 不够整帧？等下次
            if (tempBufferLen - i < frameLen) {
                break;
            }

            // 5) 校验 checksum
            if (!validateFrame1(tempBuffer + i)) {
                // 验证失败，跳过这个头，下一个 i
                continue;
            }

            // 6) 验证通过，拷贝整帧到用户缓冲
            memcpy(frameBuf, tempBuffer + i, frameLen);

            // 7) 丢弃已处理的整帧
            int remain = tempBufferLen - (i + frameLen);
            memmove(tempBuffer,
                    tempBuffer + i + frameLen,
                    remain);
            tempBufferLen = remain;
            return true;
        }
    }

    // 8) 缓冲保护，避免 tempBuffer 无限制增长
    if (tempBufferLen > TEMP_CAP/2) {
        memmove(tempBuffer,
                tempBuffer + tempBufferLen - 4,
                4);
        tempBufferLen = 4;
    }
    return false;
}


bool Lop1::validateFrame1(const uint8_t* frameBuf) const {
    // 1) 从报文 [2..3] 取 frameLen
    uint16_t frameLen = (uint16_t(frameBuf[2]) << 8)
                      | uint16_t(frameBuf[3]);
    if (frameLen != FRAME1_LEN) return false;
    //if (frameLen < MIN_FRAME || frameLen > TEMP_CAP) 
    //    return false;  // 长度不合理
    

    // 2) 校验位位于 frameLen-3
    int csIdx = frameLen - 3;

    // 3) 对除了 checksum 自身以外的所有字节累加（uint8_t 自动 mod256）
    uint8_t sum = 0;
    for (int i = 0; i < frameLen; ++i) {
        if (i == csIdx) continue;
        sum += frameBuf[i];
    }

    // 4) 加上 checksum 后，应该正好回到 0
    return uint8_t(sum + frameBuf[csIdx]) == 0;
}

void Lop1::printReceivedData1(const uint8_t* buffer) {
    cout << "Frame1: ";
    for (int i = 0; i < FRAME1_LEN; ++i) {
        printf("%02X ", buffer[i]);
    }
    cout << endl;
}


// 第二种帧接收
bool Lop1::receiveData2(uint8_t* frameBuf)
{
    int r = uart->readData(tempBuffer + tempBufferLen,
                           TEMP_CAP - tempBufferLen);
    if (r <= 0) return false;
    tempBufferLen += r;

    for (int i = 0; i <= tempBufferLen - MIN_FRAME; ++i) {
        if (tempBuffer[i] == 0xFA && tempBuffer[i+1] == 0xF5) {
            uint16_t frameLen = (uint16_t(tempBuffer[i+2]) << 8)
                              | uint16_t(tempBuffer[i+3]);
            // 只接受 32 字节这一路径
            if (frameLen != FRAME2_LEN) continue;
            if (tempBufferLen - i < frameLen) break;
            if (!validateFrame2(tempBuffer + i)) { continue; }
            memcpy(frameBuf, tempBuffer + i, frameLen);
            int rem = tempBufferLen - (i + frameLen);
            memmove(tempBuffer,
                    tempBuffer + i + frameLen,
                    rem);
            tempBufferLen = rem;
            return true;
        }
    }

    if (tempBufferLen > TEMP_CAP/2) {
        memmove(tempBuffer,
                tempBuffer + tempBufferLen - 4,
                4);
        tempBufferLen = 4;
    }
    return false;
}

bool Lop1::validateFrame2(const uint8_t* frameBuf) const {
    uint16_t frameLen = (uint16_t(frameBuf[2]) << 8)
                      | uint16_t(frameBuf[3]);
    if (frameLen != FRAME2_LEN) return false;
    int csIdx = frameLen - 3;
    uint8_t sum = 0;
    for (int i = 0; i < frameLen; ++i) {
        if (i == csIdx) continue;
        sum += frameBuf[i];
    }
    return uint8_t(sum + frameBuf[csIdx]) == 0;
}

void Lop1::printReceivedData2(const uint8_t* buffer) {
    cout << "Frame2: ";
    for (int i = 0; i < FRAME2_LEN; ++i) {
        printf("%02X ", buffer[i]);
    }
    cout << endl;
}
