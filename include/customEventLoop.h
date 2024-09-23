#ifndef EVENTLOOP_H
#define EVENTLOOP_H

#include <iostream>
#include <thread>
#include <vector>
#include <map>
#include <cassert>
#include <sys/uio.h>
#include <sys/stat.h>
#include <sys/sendfile.h>
#include "customEpoller.h"
#include "customConnector.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "customBuffer.h"
#include "data.h"
#include "utils.h"
class CustomEpoller;
class CustomConnector;
class CustomEventLoop
{

public:
    CustomEventLoop();
    ~CustomEventLoop();
    void loop();
    void addEventWaitFd(CustomConnector *customConnector);
    void modifyConnector(CustomConnector *customConnector);
    void removeConnector(CustomConnector *customConnector);
    std::thread::id getTid() const { return thread_id; }

    int HandleRead(CustomConnector *customConnector);
    
    int HandleWrite(CustomConnector *customConnector);
    int HandleClose(CustomConnector *customConnector);
    // int send(CustomConnector *customConnector);
    void stopEventLoop(CustomEventLoop*customEventLoop);

public:
    static const int kEPollTimeoutMs = 10000;
    std::unique_ptr<CustomEpoller> epoller;
    std::string thread_id_str;
    

private:
    std::thread::id thread_id;
    
    bool looping;
    bool quit;
};

#endif