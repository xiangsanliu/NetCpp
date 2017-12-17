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

    char send_char[1024];
    char received_char[1024];

    SOCKADDR_IN addr;

    addr.sin_addr.s_addr = htonl(INADDR_ANY); //设定地址，INADDR_ANY代表由系统随机指定一块网卡地址
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000); //htons 函数进行字节顺序交换

    //创建一个流套接字
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    //将套接字sock和地址addr指定的网卡绑定在一起

    if(SOCKET_ERROR == bind(serverSocket, (SOCKADDR *)&addr, sizeof(SOCKADDR_IN))){     //绑定出错
        cout<<"bind error"<<endl;
        return;
    }

    if(listen(serverSocket,10) == SOCKET_ERROR) { //开始监听
        cout<<"listen error"<<endl;
        return;
    }

    SOCKADDR_IN addrClient;
    int len = sizeof(addrClient);

    while(1) {
        //客户端连接
        SOCKET clientSocket = accept(serverSocket, (SOCKADDR *) &addrClient, &len);
        if (SOCKET_ERROR == clientSocket) {
            cout<<"accept error"<<endl;
            break;
        }

        cout<<"accept success: "<<inet_ntoa(addrClient.sin_addr);

        cout<<"Input data: ";
        cin>>send_char;
        
        //发送数据
        if(SOCKET_ERROR == send(clientSocket, send_char, sizeof(send_char), 0)){
            printf("send error");
            break;
        }

        memset(received_char, 0, sizeof(received_char));
        
        if ( recv(clientSocket, received_char, sizeof(received_char), 0) == SOCKET_ERROR) {
            cout<<"receive error"<<endl;
            break;
        }

        cout<<"received data:"<<received_char<<endl;

        closesocket(clientSocket);
    }

    closesocket(serverSocket);
    WSACleanup();
}

void doTCPClient() {

    char received_char[1024];
    char send_char[1024];
    char ip_address[30];
    cout<<"Input ip address: ";
    cin>>ip_address;
    SOCKADDR_IN addr;

    addr.sin_addr.s_addr = inet_addr(ip_address);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(5000);

    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (SOCKET_ERROR == connect(clientSocket, (SOCKADDR * ) & addr, sizeof(addr)) ){
        cout<<"connect error"<<end;
        return;
    }

    if (SOCKET_ERROR == recv(clientSocket, received_char, sizeof(received_char), 0)) {
        cout << "receive error"<<endl;
        return;
    }

    cout<<"recevied data:"<<received_char<<endl;
    cout<<"Input send data: ";
    cin>>send_char;

    if (SOCKET_ERROR == send(clientSocket, send_char, sizeof(send_char), 0)) {
        cout << "send error"<<endl;
        return;
    }

    closesocket(clientSocket);

}

#endif //NETCPP_TCP_CS_H
