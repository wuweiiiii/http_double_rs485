#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <termios.h>
#include "linux_uart.h"


LinuxUart::LinuxUart(const string &deviceName, int baudRate)
{
    fd = open(deviceName.c_str(),O_RDWR | O_NOCTTY);
    if(fd < 0){
        fprintf(stderr,"Fail to open %s,err:%s\n",deviceName.c_str(),strerror(errno));
        exit(EXIT_FAILURE);
    }

    printf("open %s success \n",deviceName.c_str());

    bool ok = defaultInit(baudRate);
    if(!ok){
        fprintf(stderr,"Fail to init baudRate:%d\n",baudRate);
        exit(EXIT_FAILURE);
    }
}

LinuxUart::~LinuxUart()
{
    close(fd);
}

bool LinuxUart::defaultInit(int baudRate)
{
    int ret;
    int err;
    struct termios tio;
    if (tcgetattr(fd, &tio) != 0) {
        std::cerr << "Failed to get serial port attributes" << std::endl;
        close(fd);
        return -1;
    }

    // 清空各类模式
    tio.c_iflag = 0;
    tio.c_oflag = 0;
    tio.c_lflag = 0;
    tio.c_cflag &= ~CSIZE;
    tio.c_cflag |= CLOCAL | CREAD; // 本地连接、允许接收
    
    // 设置波特率
    switch (baudRate){
    case 4800:
        cfsetispeed(&tio, B4800);
        cfsetospeed(&tio, B4800);
        break;
    case 9600:
        cfsetispeed(&tio, B9600);
        cfsetospeed(&tio, B9600);
        break;
    case 57600:
        cfsetispeed(&tio, B57600);
        cfsetospeed(&tio, B57600);
        break;
    case 115200:
        cfsetispeed(&tio, B115200);
        cfsetospeed(&tio, B115200);
        break;
    default:
        fprintf(stderr, "The baudrate:%d is not support\n", baudRate);
        return false;
    }

    // 设置数据位为8bit  
    tio.c_cflag |= CS8;
    // 启用奇偶校验并设置为奇校验
    tio.c_cflag |= PARENB;  // 启用奇偶校验
    tio.c_cflag |= PARODD;  // 设置为奇校验
    tio.c_cflag &= ~CSTOPB; // 1位停止位

    // 设置等待时间和最小接收字符 
    tio.c_cc[VTIME] = 0;
    tio.c_cc[VMIN] = 1;

    // 刷新串口:处理未接收字符 
    tcflush(fd, TCIOFLUSH);

    // 设置参数
    err = tcsetattr(fd, TCSANOW, &tio);//TCSANOW的意思就是配置立即生效
    if (err){
        fprintf(stderr, "Fail to tcsetattr,err:%s\n", strerror(errno));
        return false;
    }
    return true;
}

/**
 * @brief 串口读取数据,返回实际读取到的长度
 * 
 * @param buf 
 * @param size 
 * @return int 
 */
int LinuxUart::readData(uint8_t *buf, uint32_t size)
{
    int len;
    len = read(fd,buf,size);
    if(len < 0){
        fprintf(stderr, "Fail to readData,err:%s\n", strerror(errno));
        return -1;
    }

    return len;
}

/**
 * @brief 串口写入数据,返回实际写入的长度
 * 
 * @param buf 
 * @param size 
 * @return int 
 */
int LinuxUart::writeData(const uint8_t *buf, uint32_t size)
{
    int len;
    len = write(fd,buf,size);
    if(len < 0){
        fprintf(stderr, "Fail to writeData,err:%s\n", strerror(errno));
        return -1;
    }

    return len;
}

int LinuxUart::readFixLenData(uint8_t *buf, uint32_t fixLen)
{

    int n;
    int count = 0;
    while(count < fixLen){
        n = read(fd,buf + count,fixLen - count);
        if(n <= 0){
            break;
        }
        count += n;
    }

    return count != fixLen?-1:fixLen;
}
