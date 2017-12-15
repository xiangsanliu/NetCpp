//
// Created by xiang on 2017/12/15.
//

#ifndef NETCPP_TCP_CS_H
#define NETCPP_TCP_CS_H

#include <winsock2.h>
#include <cstdio>
#include <iostream>

using namespace std;

void doTCPServer() {

    u_short port = 5099;
    char buf[] = "Server: hello, I am a server.....";


    SOCKADDR_IN serv;

    serv.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //设定地址，INADDR_ANY代表由系统随机指定一块网卡地址
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port); //htons 函数进行字节顺序交换

    int addlen = sizeof(serv);

    //创建一个流套接字
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);


//    if (bind(sock, (sockaddr*) &serv, addlen)) {
//        cout<<"绑定错误"<<endl;
//    } else {
//        cout<<"服务器创建成功"<<endl;
//        listen(sock, 5);
//    }

    //将套接字sock和地址serv指定的网卡绑定在一起
    int retVal = bind(sock, (LPSOCKADDR)&serv, sizeof(SOCKADDR_IN));

    if(retVal == SOCKET_ERROR){     //绑定出错
        printf("Failed bind:%d\n", WSAGetLastError());
        return;
    }

    if(listen(sock,10) ==SOCKET_ERROR){ //开始监听
        printf("Listen failed:%d", WSAGetLastError());
        return;
    }

    SOCKADDR_IN addrClient;
    int len = sizeof(SOCKADDR);

    while(1)
    {
        //客户端连接
        SOCKET sockConn = accept(sock, (SOCKADDR *) &addrClient, &len);
        if(sockConn == SOCKET_ERROR){
            printf("Accept failed:%d", WSAGetLastError());
            break;
        }

        printf("Accept client IP:[%s]\n", inet_ntoa(addrClient.sin_addr));

        //发送数据
        int iSend = send(sockConn, buf, sizeof(buf) , 0);
        if(iSend == SOCKET_ERROR){
            printf("send failed");
            break;
        }

        char recvBuf[100];
        memset(recvBuf, 0, sizeof(recvBuf));
//      //接收数据
        recv(sockConn, recvBuf, sizeof(recvBuf), 0);
        printf("%s\n", recvBuf);

        closesocket(sockConn);
    }

    closesocket(sock);
    WSACleanup();
    system("pause");
}

#endif //NETCPP_TCP_CS_H
