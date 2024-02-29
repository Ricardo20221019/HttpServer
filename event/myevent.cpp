#include "myevent.h"

// 类外初始化静态成员
std::unordered_map<int, Request> EventBase::requestStatus;
std::unordered_map<int, Response> EventBase::responseStatus;
std::unordered_map<std::string,json> EventBase::all_msgs;


void AcceptConn::process(){
    // 接受连接
    clientAddrLen = sizeof(clientAddr);
    int accFd = accept(m_listenFd, (sockaddr*)&clientAddr, &clientAddrLen);
    if(accFd == -1){
        keepLog(  outHead("error") + "接受新连接失败" );
        return;
    }

    // 将连接设置为非阻塞
    setNonBlocking(accFd);

    // 将连接加入到监听，客户端套接字都设置为 EPOLLET 和 EPOLLONESHOT
    addWaitFd(m_epollFd, accFd, true, true);
    keepLog(  outHead("info") +"接受新连接 " + std::to_string(accFd) + " 成功" );

}


void HandleRecv::process(){
    keepLog(  outHead("info") + "开始处理客户端 " + std::to_string(m_clientFd) + " 的一个 HandleRecv 事件");
    // 获取 Request 对象，保存到m_clientFd索引的requestStatus中（没有时会自动创建一个新的）
    requestStatus[m_clientFd];

    // 读取输入，检测是否是断开连接，否则处理请求
    char buf[2048];
    int recvLen = 0; 
    
    while(1){
        // 循环接收数据，直到缓冲区读取不到数据或请求消息处理完成时退出循环
        recvLen = recv(m_clientFd, buf, 2048, 0);
        
        // 对方关闭连接，直接断开连接，设置当前状态为 HANDLE_ERROR，再退出循环
        if(recvLen == 0){
            keepLog( outHead("info") + "客户端 " + std::to_string(m_clientFd) + " 关闭连接" );
            // std::cout<<"客户端关闭连接！"<<std::endl;
            requestStatus[m_clientFd].status = HANDLE_ERROR;
            break;
        }
        // std::cout<<"读取到的数据："<<std::endl<<buf<<std::endl;
        //如果缓冲区的数据已经读完，退出读数据的状态
        if(recvLen == -1){
            if(errno != EAGAIN){    // 如果不是缓冲区为空，设置状态为错误，并退出循环
                requestStatus[m_clientFd].status = HANDLE_ERROR;
                keepLog( outHead("error") + "接收数据时返回 -1 (errno = " +std::to_string(errno)  + ")" );
                break;
            }
            // 如果是缓冲区为空，表示需要等待数据发送，由于是 EPOLLONESHOT，再退出循环，等再发来数据时再来处理
            modifyWaitFd(m_epollFd, m_clientFd, true, true, false);
            break;
        }

        // 将收到的数据拼接到之前收到的数据后面，由于在处理文件时，里面可能有 \0，所以使用 append 将 buf 内的所有字符都保存到 recvMsg 中
        requestStatus[m_clientFd].recvMsg.append(buf, recvLen);

        // 边接收数据边处理
        // 根据请求报文的状态执行操作，以下操作中，如果成功了，则解析请求报文的下个部分，如果某个部分还没有完全接收，会退出当前处理步骤，等再次收到数据后根据这次解析的状态继续处理
        
        // 保存字符串查找结果，每次查找都可以用该变量暂存查找结果
        std::string::size_type endIndex = 0;
        
        // 如果是初始状态，获取请求行
        if(requestStatus[m_clientFd].status == HANDLE_INIT){

            endIndex = requestStatus[m_clientFd].recvMsg.find("\r\n");       // 查找请求行的结束边界
            
            if(endIndex != std::string::npos){
                // 保存请求行
                requestStatus[m_clientFd].setRequestLine( requestStatus[m_clientFd].recvMsg.substr(0, endIndex + 2) ); // std::cout << requestStatus[m_clientFd].recvMsg.substr(0, endIndex + 2);
                requestStatus[m_clientFd].recvMsg.erase(0, endIndex + 2);    // 删除收到的数据中的请求行
                requestStatus[m_clientFd].status = HANDLE_HEAD;              // 将状态设置为处理消息首部
                // std::cout << outHead("info") << "处理客户端 " << m_clientFd << " 的请求行完成" << std::endl;
            }

            // 如果没有找到 \r\n，表示数据还没有接收完成，会跳回上面继续接收数据
        }
        
        // 如果是处理首部的状态，逐行解析首部字段，直至遇到空行
        if(requestStatus[m_clientFd].status == HANDLE_HEAD){
            
            std::string curLine;       // 用于暂存获取的一行数据

            while(1){
                
                endIndex = requestStatus[m_clientFd].recvMsg.find("\r\n");            // 获取一行的边界
                if(endIndex == std::string::npos){                                    // 如果没有找到边界，表示后面的数据还没有接收完整，退出循环，等待下次接收后处理
                    break;
                }

                curLine = requestStatus[m_clientFd].recvMsg.substr(0, endIndex + 2);  // 将该行的内容取出
                requestStatus[m_clientFd].recvMsg.erase(0, endIndex + 2);             // 删除收到的数据中的该行数据

                if(curLine == "\r\n"){
                    requestStatus[m_clientFd].status = HANDLE_BODY;                                       // 如果是空行，将状态修改为等待解析消息体
                    // std::cout << outHead("info") << "处理客户端 " << m_clientFd << " 的消息首部完成" << std::endl;
                    // if(requestStatus[m_clientFd].requestMethod == "POST"){
                        // std::cout << outHead("info") << "客户端 " << m_clientFd << " 发送 POST 请求，开始处理请求体" << std::endl;
                    // }
                    break;                                                                                // 退出首部字段循环
                }
                requestStatus[m_clientFd].addHeaderOpt(curLine);                      // 如果不是空行，需要将该首部保存
            }
        }

        // 如果是处理消息体的状态，根据请求类型执行特定的操作
        if(requestStatus[m_clientFd].status == HANDLE_BODY){
            // GET 操作时表示请求数据，将请求的资源路径交给 HandleSend 事件处理
            if(requestStatus[m_clientFd].requestMethod == "GET"){
                // if(requestStatus[m_clientFd])

                // 设置响应消息的资源路径，在 HandleSend 中根据请求资源构建整个响应消息并发送
               
                responseStatus[m_clientFd].resourseName = requestStatus[m_clientFd].rquestResourse;//requestStatus[m_clientFd].rquestResourse
                all_msgs[responseStatus[m_clientFd].resourseName]["GET"]="hello world！";
                modifyWaitFd(m_epollFd, m_clientFd, true, true, true);
                requestStatus[m_clientFd].status = HADNLE_COMPLATE;
                // std::cout << outHead("info") << "客户端 " << m_clientFd << " 发送 GET 请求，已将请求资源构成 Response 写事件等待发送数据" << std::endl; 
                break;
            }

            // POST 表示上传数据，执行接收数据的操作
            if(requestStatus[m_clientFd].requestMethod == "POST"){

                
                // 记录未处理的数据长度，用于当前 if 步骤处理结束时，计算处理了多少消息体数据，处理非文件时用来判断数据边界（文件使用 boundary 确定边界）
                std::string::size_type beginSize = requestStatus[m_clientFd].recvMsg.size();
                if(requestStatus[m_clientFd].msgHeader["Content-Type"] == "application/json")
                {
                    keepLog( outHead("info") + "客户端 " + std::to_string(m_clientFd) + " 的 POST 请求用于向服务器发送json数据，寻找文件头开始边界..." );
                    // endIndex = requestStatus[m_clientFd].recvMsg.find("\r\n");
                     responseStatus[m_clientFd].resourseName = requestStatus[m_clientFd].rquestResourse;//requestStatus[m_clientFd].rquestResourse
                    //  std::cout<<"查找结果："<<endIndex<<std::endl;
                    if(requestStatus[m_clientFd].recvMsg.size()==requestStatus[m_clientFd].contentLength)//
                    {
                            // std::cout<<"接收到数据："<<std::endl<<requestStatus[m_clientFd].recvMsg<<std::endl;
                            json recv_j=json::parse(requestStatus[m_clientFd].recvMsg);
                            // std::cout<<"数据转换为json格式："<<std::endl;
                            // std::cout<<recv_j<<std::endl;
                            all_msgs[responseStatus[m_clientFd].resourseName]["POST"]=recv_j;
                            // std::cout<<"查找结果："<<endIndex<<std::endl;
                            requestStatus[m_clientFd].recvMsg.clear();
                            // std::cout<<"当前长度："<<requestStatus[m_clientFd].recvMsg.size()<<std::endl;
                            requestStatus[m_clientFd].status = HADNLE_COMPLATE;
                           keepLog( outHead("info")+ "客户端 " + std::to_string(m_clientFd)+ " 的 POST 请求体接收完成！" );
                            modifyWaitFd(m_epollFd, m_clientFd, true, true, true);   // 重置可读事件和可写事件，用于发送重定向回复报文
                            // send(m_clientFd, requestStatus[m_clientFd].recvMsg.c_str(), requestStatus[m_clientFd].recvMsg.size(), 0);
                            break;
                    }
                    else
                    {
                            // 如果和边界不同，表示出错，直接返回重定向报文，重新请求文件列表
                            responseStatus[m_clientFd].resourseName = "/redirect"; 
                            modifyWaitFd(m_epollFd, m_clientFd, true, true, true);   // 重置可读事件和可写事件，用于发送重定向回复报文
                            requestStatus[m_clientFd].status = HADNLE_COMPLATE;
                            keepLog(  outHead("error") + "客户端 " + std::to_string(m_clientFd) + " 的 POST 请求体中没有找到文件头开始边界，添加重定向 Response 写事件，使客户端重定向到文件列表" );
                            break;
                    }

                }
                else{    // POST 是其他类型的数据
                    // 其他 POST 类型的数据时，直接返回重定向报文，获取文件列表
                    responseStatus[m_clientFd].resourseName = "/redirect";
                    modifyWaitFd(m_epollFd, m_clientFd, true, true, true);
                    requestStatus[m_clientFd].status = HADNLE_COMPLATE;
                    keepLog(  outHead("error") + "客户端 " + std::to_string(m_clientFd) + " 的 POST 请求中接收Content-Type不为 application/json！");
                    break;
                }
            }

        }

    }

    
    if(requestStatus[m_clientFd].status == HADNLE_COMPLATE){     // 如果请求处理完成，将该套接字对应的请求删除
        keepLog(  outHead("info") + "客户端 " +std::to_string(m_clientFd)  + " 的请求消息处理成功" );
        requestStatus.erase(m_clientFd);
    }else if(requestStatus[m_clientFd].status == HANDLE_ERROR){        
        // 请求处理错误，关闭该文件描述符，将该套接字对应的请求删除，从监听列表中删除该文件描述符
       keepLog(  outHead("error") + "客户端 " + std::to_string(m_clientFd)+  " 的请求消息处理失败，关闭连接" );
        // 先删除监听的文件描述符
        deleteWaitFd(m_epollFd, m_clientFd);
        // 再关闭文件描述符
        shutdown(m_clientFd, SHUT_RDWR);
        close(m_clientFd);
        requestStatus.erase(m_clientFd);
    }
    
}


void HandleSend::process(){
    keepLog(  outHead("info") + "开始处理客户端 "+ std::to_string(m_clientFd) +" 的一个 HandleSend 事件" );
    // 如果该套接字没有需要处理的 Response 消息，直接退出
    if(responseStatus.find(m_clientFd) == responseStatus.end()){
        keepLog( outHead("info") + "客户端 " +std::to_string(m_clientFd) + " 没有要处理的响应消息" );
        return;
    }
    
    // 根据 Response 对象的状态执行特定的处理
    // std::cout<<"存在需要处理的 Response 消息！"<<std::endl;
    // 如果处于初始状态，根据请求的文件构建不同类型的发送数据
    if(responseStatus[m_clientFd].status == HANDLE_INIT){
        // 首先分离操作方法和文件
        // std::cout<<"responseStatus[m_clientFd].resourseName："<<responseStatus[m_clientFd].resourseName<<std::endl;
        if(!responseStatus[m_clientFd].resourseName.empty()){
            // 如果是访问根目录，下面会直接返回文件列表
        }


        // 初始状态中，根据资操作确定所发送数据的内容
            // 添加状态行
            responseStatus[m_clientFd].beforeBodyMsg = getStatusLine("HTTP/1.1", "302", "Moved Temporarily");

            responseStatus[m_clientFd].msgBody=all_msgs[responseStatus[m_clientFd].resourseName].dump();
            responseStatus[m_clientFd].msgBody+="\r\n";
            responseStatus[m_clientFd].msgBodyLen=responseStatus[m_clientFd].msgBody.size();

            // 构建重定向的消息首部
            responseStatus[m_clientFd].beforeBodyMsg += getMessageHeader(std::to_string( responseStatus[m_clientFd].msgBodyLen), "json", "/", "");

            // 加入空行
            responseStatus[m_clientFd].beforeBodyMsg += "\r\n";

            responseStatus[m_clientFd].beforeBodyMsgLen = responseStatus[m_clientFd].beforeBodyMsg.size();

            // 设置标识，转换到发送数据的状态
            responseStatus[m_clientFd].bodyType = JSON_TYPE;    // 设置消息体的类型
            responseStatus[m_clientFd].status = HANDLE_HEAD;     // 设置状态为处理消息头
            responseStatus[m_clientFd].curStatusHasSendLen = 0;   // 设置当前已发送的数据长度为0
            keepLog(  outHead("info") + "客户端 " + std::to_string(m_clientFd) + " 的响应报文是重定向报文，状态行和消息首部已构建完成" );
        }
    

    while(1){
        long long sentLen = 0;
        // 发送响应消息头
        if(responseStatus[m_clientFd].status == HANDLE_HEAD){
            // 开始发送消息体之前的所有数据
            sentLen = responseStatus[m_clientFd].curStatusHasSendLen;
            sentLen = send(m_clientFd, responseStatus[m_clientFd].beforeBodyMsg.c_str() , responseStatus[m_clientFd].beforeBodyMsgLen - sentLen, 0);
            // std::cout<<"发送的响应消息头："<<sentLen;
            if(sentLen == -1) {
                if(errno != EAGAIN){
                    // 如果不是缓冲区满，设置发送失败状态，并退出循环
                    requestStatus[m_clientFd].status = HANDLE_ERROR;
                    keepLog( outHead("error") + "发送响应体和消息首部时返回 -1 (errno = " + std::to_string(errno) + ")" );
                    break;
                }
                // 如果缓冲区已满，退出循环，下面会重置 EPOLLOUT 事件，等待下次进入函数继续发送
                break;
            }
            responseStatus[m_clientFd].curStatusHasSendLen += sentLen;
            // 如果数据已经发送完成，将状态设置为发送消息体
            if(responseStatus[m_clientFd].curStatusHasSendLen >= responseStatus[m_clientFd].beforeBodyMsgLen){
                responseStatus[m_clientFd].status = HANDLE_BODY;     // 设置为正在处理消息体的状态
                responseStatus[m_clientFd].curStatusHasSendLen = 0;   // 设置已经发送的数据长度为 0
                keepLog(  outHead("info") + "客户端 " + std::to_string(m_clientFd) + " 响应消息的状态行和消息首部发送完成，正在发送消息体..." );
            }

            // 如果发送的是文件，输出提示信息
            // if(responseStatus[m_clientFd].bodyType == FILE_TYPE){
            //     std::cout << outHead("info") << "客户端 " << m_clientFd << " 请求的是文件，开始发送文件 " << responseStatus[m_clientFd].resourseName << " ..." << std::endl;
            // }
        }

        // 发送响应消息体
        if(responseStatus[m_clientFd].status == HANDLE_BODY){
            if(responseStatus[m_clientFd].bodyType == JSON_TYPE)
            {
                // 消息体为 json 数据时的发送方法
                sentLen = responseStatus[m_clientFd].curStatusHasSendLen;
                //  std::cout<<"当前sentLen："<<sentLen<<std::endl;
                sentLen = send(m_clientFd, responseStatus[m_clientFd].msgBody.c_str() + sentLen, responseStatus[m_clientFd].msgBody.size() - sentLen, 0);
                //  std::cout<<"构建的消息体："<<responseStatus[m_clientFd].msgBody<<std::endl;
                // std::cout<<"响应消息："<<responseStatus[m_clientFd].msgBody<<"     响应长度："<<sentLen<<std::endl;
                if(sentLen == -1){
                    if(errno != EAGAIN){
                        // 如果不是缓冲区满，设置发送失败状态，并退出循环
                        requestStatus[m_clientFd].status = HANDLE_ERROR;
                        keepLog(  outHead("error") + "发送 JSON 消息体时返回 -1 (errno = " +std::to_string(errno)  + ")" );
                        break;
                    }
                    
                    // 如果缓冲区已满，退出循环，下面会重置 EPOLLOUT 事件，等待下次进入函数继续发送
                    break;
                }
                responseStatus[m_clientFd].curStatusHasSendLen += sentLen;
                
                // 如果数据已经发送完成，将状态设置为发送消息体
                if(responseStatus[m_clientFd].curStatusHasSendLen >= responseStatus[m_clientFd].msgBodyLen){
                    responseStatus[m_clientFd].status = HADNLE_COMPLATE;     // 设置为正在处理消息体的状态
                    responseStatus[m_clientFd].curStatusHasSendLen = 0;   // 设置已经发送的数据长度为 0
                    keepLog(  outHead("info")+ "客户端 " + std::to_string(m_clientFd) + " 请求的是 json 文件，文件发送成功" );
                    break;
                }
            }
        }

        if(responseStatus[m_clientFd].status == HANDLE_ERROR){    // 如果是出错状态，退出 while 处理
            break;
        }
    }
    

    // 判断发送最终状态执行特定的操作
    if(responseStatus[m_clientFd].status == HADNLE_COMPLATE){
        // 完成发送数据后删除该响应
        responseStatus.erase(m_clientFd);
        modifyWaitFd(m_epollFd, m_clientFd, true, true, false);                            // 不再监听写事件
        keepLog(  outHead("info") + "客户端 " +std::to_string(m_clientFd)  + " 的响应报文发送成功" );
    }else if(responseStatus[m_clientFd].status == HANDLE_ERROR){
        // 如果发送失败，删除该响应，删除监听该文件描述符，关闭连接
        responseStatus.erase(m_clientFd);
        // 不再监听写事件
        modifyWaitFd(m_epollFd, m_clientFd, true, false, false);
        // 关闭文件描述符
        shutdown(m_clientFd, SHUT_WR);
        close(m_clientFd);
        keepLog(  outHead("error") + "客户端 " +std::to_string(m_clientFd)  + " 的响应报文发送失败，关闭相关的文件描述符" );
    }else{                      // 如果不是完成了数据传输或出错，应该重置 EPOLLSHOT 事件，保证写事件可以继续产生，继续传输数据
        modifyWaitFd(m_epollFd, m_clientFd, true, true, true);
        // 退出函数，当执行失败时或数据传输完成时才需要关闭文件
        return;
    }

    // 处理成功或非文件打开失败时需要关闭文件
    // if(responseStatus[m_clientFd].bodyType == FILE_TYPE){
    //     close(responseStatus[m_clientFd].fileMsgFd);
    // }

}

// 用于构建状态行，参数分别表示状态行的三个部分
std::string HandleSend::getStatusLine(const std::string &httpVersion, const std::string &statusCode, const std::string &statusDes){
    std::string statusLine;
    // 记录状态行相关的参数
    responseStatus[m_clientFd].responseHttpVersion = httpVersion;
    responseStatus[m_clientFd].responseStatusCode = statusCode;
    responseStatus[m_clientFd].responseStatusDes = statusDes;
    // 构建状态行
    statusLine = httpVersion + " ";
    statusLine += statusCode + " ";
    statusLine += statusDes + "\r\n";

    return statusLine;
}

// 构建头部字段：
// contentLength        : 指定消息体的长度
// contentType          : 指定消息体的类型
// redirectLoction = "" : 如果是重定向报文，可以指定重定向的地址。空字符串表示不添加该首部。
// contentRange = ""    : 如果是下载文件的响应报文，指定当前发送的文件范围。空字符串表示不添加该首部。
std::string HandleSend::getMessageHeader(const std::string contentLength, const std::string contentType, const std::string redirectLoction, const std::string contentRange){
    std::string headerOpt;
    

    // 添加消息体长度字段
    if(contentLength != ""){
        headerOpt += "Content-Length: " + contentLength + "\r\n";
    }

    // 添加消息体类型字段
    if(contentType != ""){
        if(contentType == "file"){
            headerOpt += "Content-Type: application/octet-stream\r\n";    // 发送文件时指定的类型
        }
        else if(contentType == "json"){
            headerOpt += "Content-Type: application/json\r\n";            // 发送 JSON 数据时指定的类型
        }
    }

    // 添加重定向位置字段
    if(redirectLoction != ""){
        headerOpt += "Location: " + redirectLoction + "\r\n";
    }

    // 添加文件范围的字段
    if(contentRange != ""){
        headerOpt += "Content-Range: 0-" + contentRange + "\r\n";
    }

    headerOpt += "Connection: keep-alive\r\n";

    return headerOpt;
}
