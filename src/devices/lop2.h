/*
2025/4/10
ww
*/
#ifndef _LOP2_H
#define _LOP2_H

#include "linux_uart.h"
#include <string.h>

using namespace std;

extern uint16_t calc_crc16(unsigned char *buf, int len);

class Lop2{
    public:
        Lop2(const string &deviceName, int baudRate = 9600);
        ~Lop2();
    
    private:
        LinuxUart *uart;
        string deviceName;
        int baudRate;

        static const uint8_t COMMAND[];
        static const int COMMAND_SIZE = 8;

        static const int FRAME_SIZE = 65;
        static const int DATA_SIZE = 60;

    public:
        bool initialize();

        bool sendcommand();

        bool receiveData(uint8_t* buffer);
        bool validateFrame(uint8_t* buffer);

        void printReceivedData(const uint8_t* buffer);

};


#endif