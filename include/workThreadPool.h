#ifndef WORKTHREADPOOL_H
#define WORKTHREADPOOL_H
#include <vector>
#include <thread>
#include <exception>
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "customConnector.h"
#include "data.h"
#include "utils.h"
#include "workEvent.h"
#include "myserver.h"

class MyServer;
class CustomConnector;
class HttpRequest;
class HttpResponse;

class WorkThreadPool
{
public:
    WorkThreadPool(int thread_num);
    ~WorkThreadPool();
    void submit(CustomConnector *customConnector);
    void stopWork(WorkThreadPool *workThreadPool);

private:
    int onMessage(CustomConnector *customConnector);
    int onRequest(CustomConnector *customConnector);
    void handleFailedRequest(CustomConnector *customConnector,int res);


private:
    int threadNum;
    std::vector<std::thread> threads;
    std::queue<CustomConnector *> task_queue;
    std::mutex mutex;
    std::condition_variable cond;
    bool stop;
};
#endif