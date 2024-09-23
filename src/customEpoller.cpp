#include "customEpoller.h"

CustomEpoller::CustomEpoller(CustomEventLoop *eventLoop) : owner_loop(eventLoop),
                                                           epfd(epoll_create1(0)),
                                                           revents(InitEventsSize)
{
}
CustomEpoller::~CustomEpoller()
{
    close(epfd);
}
void CustomEpoller::poll(int timeouts)
{
    int numEvents = epoll_wait(epfd, &*revents.begin(), InitEventsSize, -1);
    if (numEvents > 0)
    {

        for (int i = 0; i < numEvents; i++)
        {
            int fd = revents[i].data.fd;
            int handle_res = 0;
            if (connectorMap.find(fd) == connectorMap.end())
            {
                LOG(outHead("error") +"当前没有找到连接套接字", true, ERROR_LOG);
                continue;
            }
            CustomConnector *customConnector = connectorMap[fd];
            if (customConnector == nullptr)
                continue;
            assert(customConnector->connectFd == fd);
            if (revents[i].events & EPOLLIN)
            {
                LOG(outHead("info") +owner_loop->thread_id_str+" 监听到读事件", false, SUB_REACTOR);
                handle_res = owner_loop->HandleRead(customConnector);
            }
            else if (revents[i].events & EPOLLOUT)
            {
                LOG(outHead("info") +owner_loop->thread_id_str+" 监听到写事件", false, SUB_REACTOR);
                handle_res = owner_loop->HandleWrite(customConnector);
                owner_loop->HandleClose(customConnector);
            }
            if (handle_res ==-1)
                owner_loop->HandleClose(customConnector);
        }
    }
}
void CustomEpoller::addEpollWaitFd(CustomConnector *customConnector)
{
    epoll_event event;
    event.data.fd = customConnector->connectFd;
    event.events = customConnector->events;

    connectorMap[customConnector->connectFd] = customConnector;
    epoll_ctl(epfd, EPOLL_CTL_ADD, customConnector->connectFd, &event);
    LOG(outHead("info") +owner_loop->thread_id_str+" 向epoll中添加套接字", false, SUB_REACTOR);
}
void CustomEpoller::modifyConnector(CustomConnector *customConnector)
{
    epoll_event event;
    event.data.fd = customConnector->connectFd;
    event.events = customConnector->events;
    connectorMap[customConnector->connectFd] = customConnector;
    epoll_ctl(epfd, EPOLL_CTL_MOD, customConnector->connectFd, &event);
    LOG(outHead("info") +owner_loop->thread_id_str+" 修改正在监听的套接字的事件", false, SUB_REACTOR);
}

void CustomEpoller::removeFd(CustomConnector *customConnector)
{
    int err = epoll_ctl(epfd, EPOLL_CTL_DEL, customConnector->connectFd, NULL);
    connectorMap.erase(customConnector->connectFd);
    LOG(outHead("info") +owner_loop->thread_id_str+" 删除当前套接字", false, SUB_REACTOR);
}