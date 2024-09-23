#include "reactorThreadPool.h"

ReactorThreadPool::ReactorThreadPool(int threadNum) : m_threadNum(threadNum),
                                                      subLoops(),
                                                      tempLoop(NULL),
                                                      started(false),
                                                      next(0)
{
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&cond, NULL);
    LOG(outHead("info") + " reactor 线程池构造完成！", true, MAIN_REACTOR);
}

ReactorThreadPool::~ReactorThreadPool()
{

}
void ReactorThreadPool::start()
{
    LOG(outHead("info") + " reactor 线程池开始启动，初始化线程池数量："+std::to_string(m_threadNum), false, WORK_THREAD);
    for (int i = 0; i < m_threadNum; i++)
    {
        std::thread t(&ReactorThreadPool::eventLoopThreadRun, this);
        pthread_mutex_lock(&mutex);
        while (tempLoop == NULL)
        {
            pthread_cond_wait(&cond, &mutex);
        }
        pthread_mutex_unlock(&mutex);

        subLoops.push_back(tempLoop);
        tempLoop = NULL;
        t.detach();
    }

    started = true;
    LOG(outHead("info") + " reactor 线程池已经启动", false, WORK_THREAD);
}
void ReactorThreadPool::stop()
{
    for (int i = 0; i < m_threadNum; ++i)
    {
        CustomEventLoop *cur_loop = subLoops[i];
        cur_loop->stopEventLoop(cur_loop);
        LOG(outHead("info") + "reactor 从子线程池中修改子线程状态： "+cur_loop->thread_id_str, false, MAIN_REACTOR);
    }
}
CustomEventLoop *ReactorThreadPool::getNextLoop()
{
    assert(started);
    CustomEventLoop *selected;
    if (m_threadNum > 0)
    {
        selected = subLoops[next];
        next++;
        if (next >= m_threadNum)
        {
            next = 0;
        }
        LOG(outHead("info") + "reactor 从子线程池中拿到一个子线程 ", false, MAIN_REACTOR);
    }
    if (selected == nullptr)
    {
        LOG(outHead("error") + "reactor 分发连接时没有拿到子线程 ", false, ERROR_LOG);
    }
    return selected;
}
void ReactorThreadPool::eventLoopThreadRun()
{
    CustomEventLoop *eventLoop = new CustomEventLoop();
    pthread_mutex_lock(&mutex);
    tempLoop = eventLoop;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
    LOG(outHead("info") + " 初始化子线程"+ eventLoop->thread_id_str, false, WORK_THREAD);
    eventLoop->loop();
}
