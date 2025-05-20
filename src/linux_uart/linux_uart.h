#ifndef _LINUX_UART_HEAD_H
#define _LINUX_UART_HEAD_H

#include <iostream>
#include <stdint.h>
using namespace std;

class LinuxUart
{
    public:
        LinuxUart(const string &deviceName,int baudRate = 9600);
        ~LinuxUart();
        bool defaultInit(int baudRate);
        int readData(uint8_t * buf,uint32_t size);
        int writeData(const uint8_t * buf,uint32_t size);
        int readFixLenData(uint8_t * buf,uint32_t fixLen);
    private:
        int fd;
};


#endif