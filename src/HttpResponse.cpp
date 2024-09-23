#include "HttpResponse.h"

HttpResponse::HttpResponse()
    : statusCode(Unknown),
      statusMessage(),
      contentType(),
      body(),
      responseHeaders(),
      keepAlive(true)
{
}

void HttpResponse::reset()
{
    statusCode = Unknown;
    if (!statusMessage.empty())
        statusMessage.clear();
    if (!contentType.empty())
        contentType.clear();
    if (!body.empty())
        body.clear();
    if (!responseHeaders.empty())
        responseHeaders.clear();
    if(!resourseName.empty())
        resourseName.clear();
    keepAlive = true;
    fileSize=0;
}