#ifndef CONNECTOR_H
#define CONNECTOR_H

#include <iostream>
#include "myserver.h"
#include "customEventLoop.h"
#include "customBuffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "utils.h"
class MyServer;
class CustomEventLoop;
class CustomBuffer;
class CustomConnector
{
public:
    CustomConnector(int connectFd_, CustomEventLoop *eventLoop_, MyServer *myserver_);
    ~CustomConnector();
    
    int decodeRequest();
    int encodeResponse();
    void enableRead();
    void disableRead();
    void enableWrite();
    void disableWrite();
    void enableClose();
    bool writeEnabled();
    void update();
    void deregister();
    

private:
    void initConnection();
    

public:
    CustomEventLoop *connector_eventLoop;
    MyServer *myserver;
    int connectFd;
    int events;
    // CustomBuffer *inputBuffer;
    char* inputBuffer = new char[2048];
    CustomBuffer *outputBuffer;
    HttpRequest *httpRequest;
    HttpResponse *httpResponse;

};

#endif