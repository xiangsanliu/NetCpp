//
// Created by xiang on 2017/12/17.
//

#ifndef NETCPP_PING_AND_TRACERT_H
#define NETCPP_PING_AND_TRACERT_H

#include <winsock2.h>

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
    u_long ulDestIp = inet_addr(ip);
    if (ulDestIp == INADDR_NONE) {
        hostent * pHostent = gethostbyname(ip);
        if (pHostent) {
            ulDestIp = (* (in_addr *) pHostent->h_addr).s_addr;
        }
    }

    sockaddr destSockAddr;
    ZeroMemory(&destSockAddr, sizeof(sockaddr_in));

}

#endif //NETCPP_PING_AND_TRACERT_H
