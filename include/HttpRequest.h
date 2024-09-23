#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <utils.h>
enum MSGSTATUS
{
    HANDLE_INIT,     // 正在接收/发送头部数据（请求行、请求头）
    HANDLE_HEAD,     // 正在接收/发送消息首部
    HANDLE_BODY,     // 正在接收/发送消息体
    HADNLE_COMPLATE, // 所有数据都已经处理完成
    HANDLE_ERROR,    // 处理过程中发生错误
};

class HttpRequest
{
public:
    std::string recvMsg;
    std::string version;
    std::string method;
    std::string url;
    MSGSTATUS currentState;
    std::map<std::string, std::string> requestHeaders;
    std::string body;
    long long contentLength = 0; // 记录消息体的长度
    long long msgBodyRecvLen;    // 已经接收的消息体长度
    std::string recvFileName;
    std::map<std::string, std::string> form;
    std::string rquestResourse;

public:
    int setRequestLine(const std::string &requestLine);
    void addHeaderOpt(const std::string &headLine);
    void reset();
    bool keepAlive();
};

#endif