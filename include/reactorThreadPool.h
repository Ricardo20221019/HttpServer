/*  文件说明：
 *  1. 用于创建线程池
 *  2. 每个线程中等待事件队列中添加新事件（EventBase 指针指向的派生类）
 *  3. 有新事件时，分配给一个线程处理，线程中调用事件的 process() 方法处理该事件
 */
#ifndef THREADPOOL_H
#define THREADPOOL_H
#pragma once
#include <queue>
#include <stdexcept>
#include <sys/timerfd.h>

#include <thread>
#include <pthread.h>
#include <semaphore.h>
#include "customEventLoop.h"
class CustomEventLoop;
static int tnum = 0;
class ReactorThreadPool
{
public:
    // 初始化线程池、互斥访问时间队列的互斥量、表示队列中事件的信号量
    ReactorThreadPool(int threadNum);
    ~ReactorThreadPool();
    void start();
    void stop();

public:
    CustomEventLoop *getNextLoop();
    void eventLoopThreadRun();

private:
    int m_threadNum; 
    // CustomEventLoop *mainEventLoop;
    std::vector<CustomEventLoop *> subLoops;
    CustomEventLoop *tempLoop;
    bool started;
    int next; // 下次选择的线程
    pthread_mutex_t mutex;
    pthread_cond_t cond;

    pthread_t *m_threads; // 保存处理事件的所有线程

    // std::queue<EventBase *> m_workQueue; // 保存所有待处理的事件
    pthread_mutex_t queueLocker;         // 用于互斥访问事件队列的锁
    sem_t queueEventNum;                 // 表示队列中事件个数变化的信号量
};

#endif