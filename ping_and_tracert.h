//
// Created by xiang on 2017/12/17.
//

#ifndef NETCPP_PING_AND_TRACERT_H
#define NETCPP_PING_AND_TRACERT_H

#define ICMP_ECHO_REQUEST 8 //回显请求类型
#define DEF_ICMP_DATA_SIZE 20 //发送数据长度
#define DEF_ICMP_PACK_SIZE 32 //数据包长度
#define MAX_ICMP_PACKET_SIZE 1024 //最大数据包长度
#define DEF_ICMP_TIMEOUT 3000  //超时为3秒
#define ICMP_TIMEOUT 11 //ICMP超时报文
#define ICMP_ECHO_REPLY 0 //回显应答类型

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include <afxres.h>
#include "tool.h"

using namespace std;

//定义ICMP报头
typedef struct {
    byte type;  //类型
    byte code;  //代码
    USHORT cksum;  //校验和
    USHORT id;      //标识符
    USHORT seq;  //序列号
    int choose;     //选项
} ICMP_HEADER;

typedef struct {
    int usSeqNo ; //记录序列号
    DWORD dwRoundTripTime ; //记录当前时间
    byte ttl ; //生存时间
    in_addr dwIPaddr ; //源IP地址
} DECODE_RESULT ;

//定义IP数据报报头
typedef struct {
    UCHAR hdr_len:4;
    byte h_len_ver:4 ; //IP版本号
    byte tos ; // 服务类型
    unsigned short total_len ; //IP包总长度
    unsigned short ident ; // 标识
    unsigned short frag_and_flags ; //标志位
    byte ttl ; //生存时间
    byte proto ; //协议
    unsigned short cksum ; //IP首部校验和
    unsigned long sourceIP ; //源IP地址
    unsigned long destIP ; //目的IP地址
} IP_HEADER ;

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

bool decodeIcmpResponseTracert(char * pBuf, int iPacketSize, DECODE_RESULT & stDecodeResult) {
    //获取收到IP数据包的首部信息
    IP_HEADER * pIpHeader = (IP_HEADER *) pBuf;
    int iIpHeaderLen = pIpHeader->hdr_len*4;
    if (iPacketSize < (int) (iIpHeaderLen + sizeof(ICMP_HEADER))) {
        return false;
    }
    //指针指向ICMP报文的首地址
    ICMP_HEADER * pIcmpHeader = (ICMP_HEADER *) (pBuf+iIpHeaderLen);
    USHORT usID, usSeqNo;
    //获得的数据包的type字段为ICMP_ECHO_REPLY, 即收到一个回显应答ICMP报文
    if (pIcmpHeader->type == ICMP_ECHO_REPLY) {
        usID = pIcmpHeader->id;
        usSeqNo = ntohs(pIcmpHeader->seq);
    } else if (pIcmpHeader->type == ICMP_TIMEOUT) {
        char * pInnerIpHeader = pBuf + iIpHeaderLen + sizeof(ICMP_HEADER);
        int iInnerIpHeaderLen = ((IP_HEADER*) pInnerIpHeader)->hdr_len*4;
        ICMP_HEADER * pInnerIcmpHeader = (ICMP_HEADER * ) (pInnerIpHeader + iInnerIpHeaderLen);
        usID = pInnerIcmpHeader->id;
        usSeqNo = pIcmpHeader->seq;
    } else
        return false;

    if (usID != GetCurrentProcessId() || usSeqNo != stDecodeResult.usSeqNo)
        return false;

    //记录对方主机的IP地址以及计算往返延时RRT
    if (pIcmpHeader->type == ICMP_ECHO_REPLY || pIcmpHeader->type == ICMP_TIMEOUT) {
        stDecodeResult.dwIPaddr.s_addr = pIpHeader->sourceIP;
        stDecodeResult.dwRoundTripTime = GetTickCount() -stDecodeResult.dwRoundTripTime;
        return true;
    }

    return false;
}

bool decodeIcmpResponsePing(char * pBuf, int iPacketSize, DECODE_RESULT & stDecodeResult) {
    //获取收到IP数据包的首部信息
    IP_HEADER * pIpHeader = (IP_HEADER *) pBuf;
    int iIpHeaderLen = pIpHeader->hdr_len * 4;
    if (iPacketSize < (int) (iIpHeaderLen + sizeof(ICMP_HEADER))) {
        return false;
    }

    //指针指向ICMP报文的首地址
    ICMP_HEADER * pIcmpHeader = (ICMP_HEADER *) (pBuf+iIpHeaderLen);
    USHORT usID, usSeqNo;

    //获得的数据包的type字段为ICMP_ECHO_REPLY, 即收到一个回显应答ICMP报文
    if (pIcmpHeader->type == ICMP_ECHO_REPLY) {
        usID = pIcmpHeader->id;
        usSeqNo = ntohs(pIcmpHeader->seq);
    }

    if (usID != GetCurrentProcessId() || usSeqNo != stDecodeResult.usSeqNo)
        return false;

    //记录对方主机的IP地址以及计算往返延时RRT
    if (pIcmpHeader->type == ICMP_ECHO_REPLY) {
        stDecodeResult.ttl = pIpHeader->ttl;
        stDecodeResult.dwIPaddr.s_addr = pIpHeader->sourceIP;
        stDecodeResult.dwRoundTripTime = GetTickCount() - stDecodeResult.dwRoundTripTime;
        return true;
    }

    return false;
}

void doPing() {
    loadWinsock();
    char ip[30];
    cout<<"Ping:  ";
    cin>>ip;

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
    char icmpSendBuffer[DEF_ICMP_PACK_SIZE];

    //填充ICMP数据报各字段
    ICMP_HEADER *pIcmpHeader = (ICMP_HEADER*) icmpSendBuffer;
    pIcmpHeader->type = ICMP_ECHO_REQUEST;
    pIcmpHeader->code = 0;
    pIcmpHeader->id = (USHORT) (GetCurrentProcessId());

    memset(icmpSendBuffer+ sizeof(ICMP_HEADER), 'E', DEF_ICMP_DATA_SIZE);

    //循环发送4个请求会先icmp数据包
    DECODE_RESULT stDecodeResult;
    for (u_short i=0; i<4; i++) {
        pIcmpHeader->seq = htons(i);
        pIcmpHeader->cksum = 0;
        pIcmpHeader->cksum = generateCheckSum((USHORT*) icmpSendBuffer, sizeof(ICMP_HEADER)+DEF_ICMP_DATA_SIZE);


        stDecodeResult.usSeqNo = i;
        stDecodeResult.dwRoundTripTime = GetTickCount();

        if (sendto(sockRow, icmpSendBuffer, sizeof(icmpSendBuffer)
                , 0, (sockaddr *) &destSockAddr, sizeof(destSockAddr)) == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEHOSTUNREACH) {
                cout<<"host unreachable"<<endl;
                exit(0);
            }
        }
        sockaddr from;
        char icmpRecvBuffer[MAX_ICMP_PACKET_SIZE];
        int iFromLen = sizeof(from);
        int iReadLen;
        while (1) {
            iReadLen = recvfrom(sockRow, icmpRecvBuffer, MAX_ICMP_PACKET_SIZE, 0, (sockaddr *) &from, &iFromLen);
            if (iReadLen!=0) {
                if (decodeIcmpResponsePing(icmpRecvBuffer, sizeof(icmpRecvBuffer), stDecodeResult)) {
                    cout<<"reply from: "<<inet_ntoa(stDecodeResult.dwIPaddr)
                        <<" byte = "<<iReadLen-20
                        <<" Time = "<<stDecodeResult.dwRoundTripTime
                        <<" TTL = "<<stDecodeResult.ttl<<endl;
                }
                break;
            } else if (WSAGetLastError() == WSAETIMEDOUT) {
                cout<<"time out"<<endl;
                break;
            } else {
                cout<<"unknown error"<<endl;
                break;
            }
        }

    }

    cout<<endl<<"Ping complete."<<endl;
    closesocket(sockRow);
    WSACleanup();
}

void doTracert() {
    loadWinsock();
    char ip[30];
    cout<<"Tracert to :  ";
    cin>>ip;

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
    char icmpSendBuffer[DEF_ICMP_PACK_SIZE];

    //填充ICMP数据报各字段
    ICMP_HEADER *pIcmpHeader = (ICMP_HEADER*) icmpSendBuffer;
    pIcmpHeader->type = ICMP_ECHO_REQUEST;
    pIcmpHeader->code = 0;
    pIcmpHeader->id = (USHORT) (GetCurrentProcessId());

    memset(icmpSendBuffer+ sizeof(ICMP_HEADER), 'E', DEF_ICMP_DATA_SIZE);

    //循环发送4个请求会先icmp数据包
    int iTTL = 1;
    bool isReach = false;
    DECODE_RESULT stDecodeResult;
    for (u_short i=0;!isReach && i<30; i++, iTTL++) {
        setsockopt(sockRow, IPPROTO_IP, IP_TTL, (char *) &iTTL, sizeof(iTTL));
        pIcmpHeader->seq = htons(i);
        pIcmpHeader->cksum = 0;
        pIcmpHeader->cksum = generateCheckSum((USHORT*) icmpSendBuffer, sizeof(ICMP_HEADER)+DEF_ICMP_DATA_SIZE);


        stDecodeResult.usSeqNo = i;
        stDecodeResult.dwRoundTripTime = GetTickCount();

        if (sendto(sockRow, icmpSendBuffer, sizeof(icmpSendBuffer)
                , 0, (sockaddr *) &destSockAddr, sizeof(destSockAddr)) == SOCKET_ERROR) {
            if (WSAGetLastError() == WSAEHOSTUNREACH) {
                cout<<"host unreachable"<<endl;
                exit(0);
            }
        }
        sockaddr from;
        char icmpRecvBuffer[MAX_ICMP_PACKET_SIZE];
        int iFromLen = sizeof(from);
        int iReadLen;
        while (1) {
            iReadLen = recvfrom(sockRow, icmpRecvBuffer, MAX_ICMP_PACKET_SIZE, 0, (sockaddr *) &from, &iFromLen);
            if (iReadLen != SOCKET_ERROR) {
                //解码得到的数据包，如果解码正确则跳出接收循环发送下一个EchoRequest包
                if (decodeIcmpResponseTracert(icmpRecvBuffer, iReadLen, stDecodeResult)) {
                    if (stDecodeResult.dwIPaddr.S_un.S_addr == destSockAddr.sin_addr.S_un.S_addr) {
                        cout<<(unsigned int) stDecodeResult.ttl<<'\t'<<stDecodeResult.dwRoundTripTime<<"ms"
                            << '\t' << inet_ntoa(stDecodeResult.dwIPaddr) << endl;
                        isReach = true;
                        break;
                    }
                }
            } else if (WSAGetLastError() == WSAETIMEDOUT) {
                cout<<iTTL<<"\t"<<"time out"<<endl;
                break;
            } else {
                cout<<"unknown error"<<endl;
                break;
            }
        }

    }

    cout<<endl<<"Ping complete."<<endl;
    closesocket(sockRow);
    WSACleanup();
}

#endif //NETCPP_PING_AND_TRACERT_H
