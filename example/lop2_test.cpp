#include <iostream>
#include <thread>
#include <chrono>

#include "lop2.h"
#include "lop2_frame.h"
#include "lop2_database.h"

using namespace std;

int main() {
    // 设备名称和波特率
    std::string deviceName = "/dev/ttyS3"; // 根据实际情况修改
    int baudRate = 9600;

    // 创建 Lop2 对象
    Lop2 lop2(deviceName, baudRate);

    // 初始化设备
    if (!lop2.initialize()) {
        std::cerr << "Failed to initialize Lop2 device." << std::endl;
        return 1;
    }

    //初始化数据库
    LOP2Database lop2database("/media/udisk0/test.db");
    lop2database.frame_init();

    //uint8_t buffer[65];

    // 创建帧解析器对象
    LOP2FrameParser parser;
    // 用于存储解析结果的结构体
    LOP2FrameData frameData; 

    // 循环发送命令和接收数据
    while (true) {
        // 发送命令
        if (!lop2.sendcommand()) {
            std::cerr << "Failed to send command." << std::endl;
            return 1;
        }

        // 接收数据
        uint8_t buffer[65];
        if (!lop2.receiveData(buffer)) {
            std::cerr << "Failed to receive data." << std::endl;
            return 1;
        }

        // 打印接收到的数据
        //lop2.printReceivedData(buffer);

        // 验证帧的有效性
        if (lop2.validateFrame(buffer)) {
            std::cout << "Received frame is valid." << std::endl;
            // 解析有效帧
            if (parser.parse(buffer, frameData)) {
                // 打印解析结果
                /*
                std::cout << "RPM: " << frameData.rpm << " rpm" << std::endl;
                std::cout << "Runtime: " << frameData.runtime << " h" << std::endl;
                std::cout << "Inside Air Temp: " << frameData.insideairtemp << " ℃" << std::endl;
                std::cout << "Oil Temp: " << frameData.oiltemp << " ℃" << std::endl;
                std::cout << "Fresh Water Temp: " << frameData.freashwatertemp << " ℃" << std::endl;
                std::cout << "A Row Temp: " << frameData.Arowtemp << " ℃" << std::endl;
                std::cout << "B Row Temp: " << frameData.Browtemp << " ℃" << std::endl;
                std::cout << "U Phase Temp: " << frameData.Uphasetemp << " ℃" << std::endl;
                std::cout << "V Phase Temp: " << frameData.Vphasetemp << " ℃" << std::endl;
                std::cout << "W Phase Temp: " << frameData.Wphasetemp << " ℃" << std::endl;   
                std::cout << "Front Bearing Temp: " << frameData.frontbearingtemp << " ℃" << std::endl;
                std::cout << "Rear Bearing Temp: " << frameData.rearbearingtemp << " ℃" << std::endl;            
                std::cout << "Inlet Air Temp: " << frameData.inletairtemp << " ℃" << std::endl;
                std::cout << "Outlet Air Temp: " << frameData.outletairtemp << " ℃" << std::endl;
                std::cout << "Oil Pressure: " << frameData.oilpressure << " MPa" << std::endl;
                std::cout << "Air Pressure: " << frameData.airpressure << " MPa" << std::endl;
                std::cout << "Fuel Pressure: " << frameData.fuelpressure << " MPa" << std::endl;

                // 打印激活的报警信息
                std::cout << "Active Alarms:" << std::endl;
                for (const auto& alarm : frameData.activeAlarms) {
                    std::cout << "warning:" << alarm << std::endl;
                }
                */
                // 写入数据库
                long rawId = lop2database.frame_insert(frameData, 65);
                if (rawId == -1) {
                    std::cerr << "Failed to insert raw frame." << std::endl;
                    continue;
                }
            } else {
                std::cerr << "Failed to parse frame." << std::endl;
            }
        } else {
            std::cout << "Received frame is invalid." << std::endl;
        }

        // 等待 500ms
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    return 0;
}