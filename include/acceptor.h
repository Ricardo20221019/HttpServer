#ifndef ACCEPTOR_H
#define ACCEPTOR_H
#pragma once
#include <signal.h>
#include <sys/types.h>
#include <stdexcept>
#include <errno.h>
#include <signal.h>
#include <atomic>
#include <unistd.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h> 

#include "utils.h"


class Acceptor
{
public:
    Acceptor(int port,const char *ip=nullptr);
    int acceptClient();

public:
    int listen_port;
    int listen_fd;


private:
    sockaddr_in m_serverAddr;         // 服务端套接字绑定的地址信息 
    static const int MaxListenFd=100;
};



#endif