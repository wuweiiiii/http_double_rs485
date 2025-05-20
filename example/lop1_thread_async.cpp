#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <memory>
#include "lop1.h"
#include "lop1_frame1.h"
#include "lop1_frame2.h"
#include "lop1_database_fast.h"

std::mutex db_mutex;  //全局锁保护数据库写入


enum FrameType { FRAME1, FRAME2 };

struct FrameTask {
    FrameType type;
    std::vector<uint8_t> data;
};

class FrameQueue {
    public:
        void push(FrameTask task) {
            std::unique_lock<std::mutex> lock(mutex_);
            queue_.push(std::move(task));
            cond_.notify_one();
        }

        FrameTask pop() {
            std::unique_lock<std::mutex> lock(mutex_);
            cond_.wait(lock, [&]{ return !queue_.empty(); });
            FrameTask task = std::move(queue_.front());
            queue_.pop();
            return task;
        }

        bool empty() {
            std::unique_lock<std::mutex> lock(mutex_);
            return queue_.empty();
        }


    private:
        std::queue<FrameTask> queue_;
        std::mutex mutex_;
        std::condition_variable cond_;
};

void receiveThread(Lop1& lop, FrameType type, size_t frameLen, FrameQueue& queue) {
    while (true) {
        std::vector<uint8_t> buffer(frameLen);
        bool ok = false;

        if (type == FRAME1) {
            ok = lop.receiveData1(buffer.data());
        } else if (type == FRAME2) {
            ok = lop.receiveData2(buffer.data());
        }

        if (ok) {
            //std::cout << "[Receive] Frame " << (type == FRAME1 ? "1" : "2") << " received." << std::endl;
            queue.push({type, std::move(buffer)}); // move buffer, avoid shallow copy
        } else {
            //std::cerr << "[Receive] Frame " << (type == FRAME1 ? "1" : "2") << " failed to receive." << std::endl;
        }

        // 可选延时防止过载
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}


void dbThread(FrameQueue& queue, LOP1Database& db) {
    LOP1Frame1Parser parser1;
    LOP1Frame2Parser parser2;
    std::vector<FrameTask> batch;
    auto lastFlush = std::chrono::steady_clock::now();

    while (true) {
        // 阻塞获取第一条
        FrameTask first = queue.pop();
        batch.push_back(std::move(first));

        // 聚合更多数据（非阻塞）
        while (batch.size() < 50) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            if (!queue.empty()) {
                batch.push_back(queue.pop());
            } else {
                break;
            }
        }

        db.beginTransaction();
        for (const auto& task : batch) {
            if (task.type == FRAME1) {
                LOP1Frame1Data data1;
                if (parser1.parse(task.data.data(), data1)) {
                    db.frame1_insert(data1, task.data.size());
                }
            } else if (task.type == FRAME2) {
                LOP1Frame2Data data2;
                if (parser2.parse(task.data.data(), data2)) {
                    db.frame2_insert(data2, task.data.size());
                }
            }
        }
        db.commitTransaction();
        batch.clear();
    }
}

int main() {
    Lop1 lop1a("/dev/ttyS7", 9600);
    if (!lop1a.initialize()) {
        std::cerr << "Failed to initialize lop1a" << std::endl;
        return -1;
    }

    Lop1 lop1b("/dev/ttyS8", 9600);
    if (!lop1b.initialize()) {
        std::cerr << "Failed to initialize lop1b" << std::endl;
        return -1;
    }

    LOP1Database db("/userdata/sqlite/lop1.db");
    db.frame1_init();
    db.frame2_init();

    FrameQueue queue;

    std::cout << "Initialization successful, start accepting" << std::endl;

    std::thread recv1(receiveThread, std::ref(lop1a), FRAME1, 35, std::ref(queue));
    std::thread recv2(receiveThread, std::ref(lop1b), FRAME2, 32, std::ref(queue));
    std::thread writer(dbThread, std::ref(queue), std::ref(db));

    recv1.join();
    recv2.join();
    writer.join();

    return 0;
}
