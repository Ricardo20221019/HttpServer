#include "customConnector.h"

CustomConnector::CustomConnector(int connectFd_, CustomEventLoop *eventLoop_, MyServer *myserver_) : connectFd(connectFd_),
                                                                                                     connector_eventLoop(eventLoop_),
                                                                                                     myserver(myserver_),
                                                                                                     events(EPOLLIN),
                                                                                                    //  inputBuffer(new CustomBuffer()),
                                                                                                     outputBuffer(new CustomBuffer()),
                                                                                                     httpRequest(new HttpRequest()),
                                                                                                     httpResponse(new HttpResponse())

{
    initConnection();
}
CustomConnector::~CustomConnector()
{
    delete httpRequest;
    delete httpResponse;
    httpRequest = nullptr;
    httpResponse = nullptr;
    int err = close(connectFd);
    if (err == 0)
    {
        LOG(outHead("info") + "closed "+std::to_string(connectFd) , false, WORK_THREAD);
    }
    else
    {
        LOG(outHead("error") + "failed to close "+std::to_string(connectFd)+" , err=" + std::to_string(err)  , false, ERROR_LOG);
    }
    // delete inputBuffer;
    // inputBuffer = nullptr;
    delete outputBuffer;
    outputBuffer = nullptr;
    LOG(outHead("info") + "customconnector析构成功 " , false, WORK_THREAD);
}
int CustomConnector::decodeRequest()
{
    int ok = 1; // TODO：错误处理
    int crlf = 0;
    bool post = false;
    // std::string s(inputBuffer->dataBegin(), inputBuffer->readableSize());
    // std::cout << "inputBuffer->dataBegin():" << std::endl
    //           << inputBuffer->dataBegin() << std::endl;
    // while (httpRequest->currentState != HADNLE_COMPLATE)
    // {
    //     if (httpRequest->currentState == HANDLE_INIT)
    //     {
    //         crlf = s.find("\r\n");
    //         int pos1 = s.find(" ");
    //         httpRequest->method = s.substr(0, pos1);
    //         if (httpRequest->method == "POST") {
    //             post = true;
    //         }
    //         int pos2 = s.find(" ", pos1+1);
    //         httpRequest->url = s.substr(pos1+1, pos2-pos1-1);
    //         httpRequest->version = s.substr(pos2+1, crlf-pos2-1);
    //         httpRequest->currentState = HANDLE_HEAD;
    //     }
    //     if (httpRequest->currentState == HANDLE_HEAD) //header中分号后要加一个空格
    //     {
    //         while (true)
    //         {
    //             int pos1 = s.find(":", crlf+2);
    //             if (pos1 == std::string::npos)
    //                 break;
    //             std::string key = s.substr(crlf+2, pos1-crlf-2);
    //             crlf = s.find("\r\n", pos1+2);
    //             if(crlf==std::string::npos)
    //             {
    //                 std::cout<<"找不到回车了"<<std::endl;
    //                 break;
    //             }
    //             std::cout<<"当前找到回车的位置："<<crlf<<std::endl;
    //             std::string value = s.substr(pos1+2, crlf-pos1-2);
    //             httpRequest->requestHeaders[key] = value;
    //         }
    //         httpRequest->currentState = HANDLE_BODY;
    //     }
    //     if (httpRequest->currentState == HANDLE_BODY)
    //     {
    //         if (post) {
    //             httpRequest->body = s.substr(crlf + 4, std::stoi(httpRequest->requestHeaders["Content-Length"]));
    //         }
    //         httpRequest->currentState = HADNLE_COMPLATE;
    //     }
    // }
    LOG(outHead("info") + "解析结束 " , false, WORK_THREAD);
    return ok;
}

int CustomConnector::encodeResponse()
{
    std::string statusLine = "HTTP/1.1 " + std::to_string(httpResponse->statusCode) + " " + httpResponse->statusMessage + "\r\n";
    outputBuffer->appendString(statusLine);
    std::string contentLen = std::to_string(httpResponse->body.size());
   
    if (httpResponse->responseHeaders["Content-Type"] == "multipart/form-data")
    {
        httpResponse->responseHeaders["Content-Type"] = "application/octet-stream";
        contentLen = std::to_string(httpResponse->fileSize);
    }
    outputBuffer->appendString("Content-Length: " + contentLen + "\r\n");
    
    if (!httpResponse->keepAlive)
    {
        outputBuffer->appendString("Connection: close\r\n");
    }
    else
    {
        outputBuffer->appendString("Connection: Keep-Alive\r\n");
    }
   
    for (auto it = httpResponse->responseHeaders.begin(); it != httpResponse->responseHeaders.end(); it++)
    {
        outputBuffer->appendString(it->first + ": " + it->second + "\r\n");
    }
    
    outputBuffer->appendString("\r\n");
    outputBuffer->appendString(httpResponse->body);
    return 0;
}

void CustomConnector::initConnection()
{
    LOG(outHead("info") + "添加连接到子线程 " , false, WORK_THREAD);
    connector_eventLoop->addEventWaitFd(this);
}

// 启用读事件
void CustomConnector::enableRead()
{
    events |= EPOLLIN;
    update();
}
// 禁用读事件
void CustomConnector::disableRead()
{
    events &= ~EPOLLIN;
    update();
}
// 启动写事件
void CustomConnector::enableWrite()
{
    events |= EPOLLOUT;
    update();
}
// 禁用写事件
void CustomConnector::disableWrite()
{
    events &= ~EPOLLOUT;
    update();
}
void CustomConnector::enableClose()
{
    events |= EPOLLHUP;
    update();
}
// 检查写事件是否已启用
bool CustomConnector::writeEnabled()
{
    return events & EPOLLOUT;
}

void CustomConnector::update()
{
    connector_eventLoop->modifyConnector(this);
}
// 注销客户端连接
void CustomConnector::deregister()
{
    connector_eventLoop->removeConnector(this);
}
