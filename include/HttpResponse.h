#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <string>
#include <vector>
#include <map>


enum HttpStatusCode {
    Unknown,    //未知状态码，未初始化或无法识别
    OK = 200,
    MovedPermanently = 301,     //永久重定向码，表示请求的资源已被永久移动到新的 URI，客户端应使用新的 URI 重新发起请求
    BadRequest = 400,   //客户端错误状态码，表示服务器无法理解请求，通常是由于请求语法错误或请求无效
    NotFound = 404,     //未找到状态码，表示服务器找不到请求的资源
    InternalServerError = 500   //服务端内部错误
};

class HttpResponse
{
public:
    HttpStatusCode statusCode;
    std::string statusMessage;
    std::string contentType;
    std::string body;
    unsigned long fileSize;
    std::map<std::string, std::string> responseHeaders;
    bool keepAlive;
    std::string resourseName;
    int fileMsgFd=-1;
public:
    HttpResponse();
    void reset();
};




#endif