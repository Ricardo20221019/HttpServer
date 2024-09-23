#ifndef READ_WRITE_LOCK_H
#define READ_WRITE_LOCK_H
#pragma once
#include <mutex>
#include <condition_variable>

class ReadWriteLock
{
private:
    int readWaiting = 0;  // 等待读
    int writeWaiting = 0; // 等待写
    int reading = 0;      // 正在读
    int writing = 0;      // 正在写
    std::mutex mx;
    std::condition_variable cond; 
    bool preferWriter; // 默认偏向读
public:
    ReadWriteLock(bool isPreferWriter = false) : preferWriter(isPreferWriter) {}
    void readLock();
    void writeLock();
    void readUnLock();
    void writeUnLock();
};

#endif