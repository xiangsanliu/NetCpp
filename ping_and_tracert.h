//
// Created by xiang on 2017/12/17.
//

#ifndef NETCPP_PING_AND_TRACERT_H
#define NETCPP_PING_AND_TRACERT_H
#define DEF_ICMP_TIMEOUT 3000

#include <winsock2.h>
#include <iostream>

using namespace std;

//生成校验和
USHORT generateCheckSum(USHORT * pBuf, int iSize) {
    ULONG cksum = 0;
    while (iSize > 1) {
        cksum += *pBuf ++;
        iSize -= sizeof(USHORT);
    }

    if (iSize) {
        cksum += * (UCHAR *) pBuf;
    }
    cksum = (cksum >> 16) + (cksum & 0xffff);
    cksum += (cksum >> 16);
    return USHORT (~cksum);
}

void doPing() {
    char ip[30];

    //获取IP地址
    u_long ulDestIp = inet_addr(ip);
    if (ulDestIp == INADDR_NONE) {
        hostent * pHostent = gethostbyname(ip);
        if (pHostent) {
            ulDestIp = (* (in_addr *) pHostent->h_addr).s_addr;
        }
    }

    //填充目的Socket地址
    SOCKADDR_IN destSockAddr;
    ZeroMemory(&destSockAddr, sizeof(sockaddr_in));
    destSockAddr.sin_family = AF_INET;
    destSockAddr.sin_addr.s_addr = ulDestIp;

    //使用ICMP协议创建Raw Socket
    SOCKET sockRow = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);

    int iTimeout = DEF_ICMP_TIMEOUT;
    if (setsockopt(sockRow, SOL_SOCKET, SO_RCVTIMEO, (char *) &iTimeout, sizeof(iTimeout)) == SOCKET_ERROR) {
        cout<<"set parm error"<<endl;
        return;
    }
    if (setsockopt(sockRow, SOL_SOCKET, SO_SNDTIMEO, (char *) &iTimeout, sizeof(iTimeout)) == SOCKET_ERROR) {
        cout<<"set parm error"<<endl;
        return;
    }

    //填充ICMP数据报各字段

}

#endif //NETCPP_PING_AND_TRACERT_H
