//
// Created by xiang on 2017/12/17.
//

#ifndef NETCPP_PING_AND_TRACERT_H
#define NETCPP_PING_AND_TRACERT_H
#define ICMP_ECHO_REQUEST 8 //������������
#define DEF_ICMP_DATA_SIZE 20 //�������ݳ���
#define DEF_ICMP_PACK_SIZE 32 //���ݰ�����
#define MAX_ICMP_PACKET_SIZE 1024 //������ݰ�����
#define DEF_ICMP_TIMEOUT 3000  //��ʱΪ3��
#define ICMP_TIMEOUT 11 //ICMP��ʱ����
#define ICMP_ECHO_REPLY 0 //����Ӧ������

#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <iomanip>
#include <cstdio>
#include "tool.h"

using namespace std;


//����ICMP��ͷ
typedef struct {
    byte type;  //����
    byte code;  //����
    USHORT cksum;  //У���
    USHORT id;      //��ʶ��
    USHORT seq;  //���к�
    int choose;     //ѡ��
} ICMP_HEADER;

typedef struct {
    int usSeqNo ; //��¼���к�
    DWORD dwRoundTripTime ; //��¼��ǰʱ��
    byte ttl ; //����ʱ��
    in_addr dwIPaddr ; //ԴIP��ַ
} DECODE_RESULT ;

//����IP���ݱ���ͷ
typedef struct {
    UCHAR hdr_len:4;
    byte h_len_ver:4 ; //IP�汾��
    byte tos ; // ��������
    unsigned short total_len ; //IP���ܳ���
    unsigned short ident ; // ��ʶ
    unsigned short frag_and_flags ; //��־λ
    byte ttl ; //����ʱ��
    byte proto ; //Э��
    unsigned short cksum ; //IP�ײ�У���
    unsigned long sourceIP ; //ԴIP��ַ
    unsigned long destIP ; //Ŀ��IP��ַ
} IP_HEADER ;

//����У���
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
    //��ȡ�յ�IP���ݰ����ײ���Ϣ
    IP_HEADER * pIpHeader = (IP_HEADER *) pBuf;
    int iIpHeaderLen = pIpHeader->hdr_len*4;
    if (iPacketSize < (int) (iIpHeaderLen + sizeof(ICMP_HEADER))) {
        return false;
    }
    cout<<"are you ok?"<<endl;
    //ָ��ָ��ICMP���ĵ��׵�ַ
    ICMP_HEADER * pIcmpHeader = (ICMP_HEADER *) (pBuf+iIpHeaderLen);
    USHORT usID, usSeqNo;
    cout<<"are you ok?1"<<endl;
    //��õ����ݰ���type�ֶ�ΪICMP_ECHO_REPLY, ���յ�һ������Ӧ��ICMP����
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

    cout<<"are you ok?2"<<endl;
    if (usID != GetCurrentProcessId() || usSeqNo != stDecodeResult.usSeqNo)
        return false;

    //��¼�Է�������IP��ַ�Լ�����������ʱRRT
    if (pIcmpHeader->type == ICMP_ECHO_REPLY || pIcmpHeader->type == ICMP_TIMEOUT) {
        stDecodeResult.dwIPaddr.s_addr = pIpHeader->sourceIP;
        stDecodeResult.dwRoundTripTime = GetTickCount() -stDecodeResult.dwRoundTripTime;
        if (stDecodeResult.dwRoundTripTime)
            cout<<setw(6)<<stDecodeResult.dwRoundTripTime<<"ms"<<flush;
        else
            cout<<setw(6)<<"<1"<<"ms"<<flush;
        return true;
    }

    return false;
}

bool decodeIcmpResponsePing(char * pBuf, int iPacketSize, DECODE_RESULT & stDecodeResult) {
    //��ȡ�յ�IP���ݰ����ײ���Ϣ
    IP_HEADER * pIpHeader = (IP_HEADER *) pBuf;
    int iIpHeaderLen = pIpHeader->hdr_len * 4;
    if (iPacketSize < (int) (iIpHeaderLen + sizeof(ICMP_HEADER))) {
        return false;
    }

    //ָ��ָ��ICMP���ĵ��׵�ַ
    ICMP_HEADER * pIcmpHeader = (ICMP_HEADER *) (pBuf+iIpHeaderLen);
    USHORT usID, usSeqNo;

    //��õ����ݰ���type�ֶ�ΪICMP_ECHO_REPLY, ���յ�һ������Ӧ��ICMP����
    if (pIcmpHeader->type == ICMP_ECHO_REPLY) {
        usID = pIcmpHeader->id;
        usSeqNo = ntohs(pIcmpHeader->seq);
    }

    if (usID != GetCurrentProcessId() || usSeqNo != stDecodeResult.usSeqNo)
        return false;

    //��¼�Է�������IP��ַ�Լ�����������ʱRRT
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

    //��ȡIP��ַ
    u_long ulDestIp = inet_addr(ip);
    if (ulDestIp == INADDR_NONE) {
        hostent * pHostent = gethostbyname(ip);
        if (pHostent) {
            ulDestIp = (* (in_addr *) pHostent->h_addr).s_addr;
        }
    }

    //���Ŀ��Socket��ַ
    SOCKADDR_IN destSockAddr;
    ZeroMemory(&destSockAddr, sizeof(sockaddr_in));
    destSockAddr.sin_family = AF_INET;
    destSockAddr.sin_addr.s_addr = ulDestIp;

    //ʹ��ICMPЭ�鴴��Raw Socket
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

    //���ICMP���ݱ����ֶ�
    ICMP_HEADER *pIcmpHeader = (ICMP_HEADER*) icmpSendBuffer;
    pIcmpHeader->type = ICMP_ECHO_REQUEST;
    pIcmpHeader->code = 0;
    pIcmpHeader->id = (USHORT) (GetCurrentProcessId());

    memset(icmpSendBuffer+ sizeof(ICMP_HEADER), 'E', DEF_ICMP_DATA_SIZE);

    //ѭ������4���������icmp���ݰ�
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
            cout<<"iReadLen"<<iReadLen<<endl;
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
    cout<<"Ping:  ";
    cin>>ip;

    //��ȡIP��ַ
    u_long ulDestIp = inet_addr(ip);
    if (ulDestIp == INADDR_NONE) {
        hostent * pHostent = gethostbyname(ip);
        if (pHostent) {
            ulDestIp = (* (in_addr *) pHostent->h_addr).s_addr;
        }
    }

    //���Ŀ��Socket��ַ
    SOCKADDR_IN destSockAddr;
    ZeroMemory(&destSockAddr, sizeof(sockaddr_in));
    destSockAddr.sin_family = AF_INET;
    destSockAddr.sin_addr.s_addr = ulDestIp;

    //ʹ��ICMPЭ�鴴��Raw Socket
    SOCKET sockRow = WSASocket(AF_INET, SOCK_RAW, IPPROTO_ICMP, NULL, 0, WSA_FLAG_OVERLAPPED);
    int iTimeout = DEF_ICMP_TIMEOUT;
    if (setsockopt(sockRow, SOL_SOCKET, SO_RCVTIMEO, (char *) &iTimeout, sizeof(iTimeout)) == SOCKET_ERROR) {
        cout<<"set parm error"<<endl;
        closesocket(sockRow);
        WSACleanup();
        return;
    }

    if (setsockopt(sockRow, SOL_SOCKET, SO_SNDTIMEO, (char *) &iTimeout, sizeof(iTimeout)) == SOCKET_ERROR) {
        cout<<"set parm error"<<endl;
        closesocket(sockRow);
        WSACleanup();
        return;
    }

    //�������ͺͽ��ܻ�����
    char icmpSendBuffer[DEF_ICMP_PACK_SIZE];
    memset(icmpSendBuffer+ sizeof(ICMP_HEADER), 'E', DEF_ICMP_DATA_SIZE);

    //���ICMP���ݱ����ֶ�
    ICMP_HEADER *pIcmpHeader = (ICMP_HEADER*) icmpSendBuffer;
    pIcmpHeader->type = ICMP_ECHO_REQUEST;
    pIcmpHeader->code = 0;
    pIcmpHeader->id = (USHORT) (GetCurrentProcessId());

    //��ʼ̽��·��
    DECODE_RESULT stDecodeResult;
    BOOL bReachDestHost = FALSE;
    USHORT usSeqNo = 0;
    int iTTL = 1;
    int iMaxLoop = 30;


    while (!bReachDestHost && iMaxLoop--) {
        //����IP���ݱ���ͷ��ttl�ֶ�
        setsockopt(sockRow, IPPROTO_IP, IP_TTL, (char *) &iTTL, sizeof(iTTL));
        cout<<setw(3)<<iTTL<<flush;

        ((ICMP_HEADER *) icmpSendBuffer)->seq = htons(usSeqNo++);
        ((ICMP_HEADER *) icmpSendBuffer)->cksum = generateCheckSum((USHORT *) icmpSendBuffer, sizeof(ICMP_HEADER)+DEF_ICMP_DATA_SIZE);

        stDecodeResult.usSeqNo = ((ICMP_HEADER *) icmpSendBuffer)->seq;
        stDecodeResult.dwRoundTripTime = GetTickCount();

        if (sendto(sockRow, icmpSendBuffer, sizeof(icmpSendBuffer)
                , 0, (sockaddr *) &destSockAddr, sizeof(destSockAddr) )==SOCKET_ERROR) {
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
            iReadLen = recvfrom(sockRow, icmpRecvBuffer, MAX_ICMP_PACKET_SIZE
                    , 0, (sockaddr *) &from, &iFromLen);
            cout<<"iReadLen"<<iReadLen<<endl;
            if (iReadLen!=SOCKET_ERROR) {
                if (decodeIcmpResponseTracert(icmpRecvBuffer
                        , sizeof(icmpRecvBuffer), stDecodeResult)) {
                    if (stDecodeResult.dwIPaddr.s_addr == destSockAddr.sin_addr.s_addr)
                        bReachDestHost = TRUE;
                    cout<<"\t"<<inet_ntoa(stDecodeResult.dwIPaddr)<<endl;
                    break;
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
        iTTL ++;

    }


    cout<<endl<<"Ping complete."<<endl;
    closesocket(sockRow);
    WSACleanup();


}

#endif //NETCPP_PING_AND_TRACERT_H
