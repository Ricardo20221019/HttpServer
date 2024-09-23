#include "read_write_lock.h"

void ReadWriteLock::readLock()
{
    std::unique_lock<std::mutex> lock(mx);
    ++readWaiting;
    //线程阻塞至：没有写锁 && （读优先 || 没有等待写） 
    cond.wait(lock, [&]()
              { return writing <= 0 && (!preferWriter || writeWaiting <= 0); });
    ++reading;
    --readWaiting;
}

void ReadWriteLock::writeLock()
{
    std::unique_lock<std::mutex> lock(mx);
    ++writeWaiting;
    cond.wait(lock, [&]()
              { return reading <= 0 && writing <= 0; });
    ++writing;
    --writeWaiting;
}

void ReadWriteLock::readUnLock()
{
    std::unique_lock<std::mutex> lock(mx);
    --reading;
    // 当前没有读者时，唤醒一个写者
    if (reading <= 0)
        cond.notify_one();
}

void ReadWriteLock::writeUnLock()
{
    std::unique_lock<std::mutex> lock(mx);
    --writing;
    // 唤醒所有读者、写者
    cond.notify_all();
}
