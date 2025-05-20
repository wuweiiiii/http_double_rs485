#include <iostream>
#include <string.h>
#include "lop2.h"

using namespace std;

Lop2::Lop2(const string& deviceName,int baudRate):deviceName(deviceName),baudRate(baudRate){
    uart = new LinuxUart(deviceName, baudRate);
}

Lop2::~Lop2(){
    delete uart;
}

bool Lop2::initialize(){
    return uart->defaultInit(baudRate);
}
//发送以下数据01 03 00 00 00 1E C5 C2

const uint8_t Lop2::COMMAND[] = {0x01, 0x03, 0x00, 0x00, 0x00, 0x1E, 0xC5, 0xC2};
bool Lop2::sendcommand(){
    return uart->writeData(COMMAND,COMMAND_SIZE);
}

bool Lop2::receiveData(uint8_t* buffer){
    return uart->readFixLenData(buffer,FRAME_SIZE);                
}

//校验帧是否正确
bool Lop2::validateFrame(uint8_t* buffer) {
    // 计算接收到的数据的 CRC 校验值，不包括最后两个字节
    uint16_t calculatedCrc = calc_crc16(buffer, FRAME_SIZE - 2);
    // 打印计算得到的 CRC 校验值
    //cout << "Calculated CRC: " <<  calculatedCrc << endl;
    // 提取接收到的数据的最后两个字节作为 CRC 校验值
    uint16_t receivedCrc = (buffer[FRAME_SIZE - 2] << 8) | buffer[FRAME_SIZE - 1];
    // 比较计算得到的 CRC 校验值和接收到的 CRC 校验值
    if (calculatedCrc == receivedCrc) {
        cout << "Valid Frame Received" << endl;
        return true;
    } else {
        cerr << "Invalid CRC Checksum!" << endl;
        return false;
    }
}

void Lop2::printReceivedData(const uint8_t* buffer) {
    cout << "Received data1: ";
    for (int i = 0; i < FRAME_SIZE; ++i) {
        printf("%02X ", buffer[i]);
    }
    cout << endl;
}
