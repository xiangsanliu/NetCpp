//
// Created by xiang on 2017/12/19.
//

#ifndef NETCPP_NDS_SERVER_H
#define NETCPP_NDS_SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <winsock2.h>
#include <Iphlpapi.h>
#include <windows.h>
#include <string.h>
#include <iptypes.h>

#include "tool.h"

using namespace std;

#pragma pack(2)


#define PORT 53
#define TIMEOUT 3000

//DNS报文首部
typedef struct {
    u_short id;
    u_short flags;
    u_short questNum;
    u_short answerNum;
    u_short authorNum;
    u_short additionNum;
} DNSHDR, *PDNSHDR;

//DNS报文查询记录
typedef struct {
    u_short type;
    u_short queryclass;
}QUERYHDR, *PQUERYHDDR;

//DNS报文应答记录
typedef struct {
    u_short type;
    u_short classes;
    u_long ttl;
    u_short length;
}RESOPNSE, *PRESPONSE;


int genDNSPack(PDNSHDR pdnshdr, PQUERYHDDR pqueryhddr, char * hostname, char * dnsSendBuff) {
    if (!(strcmp(hostname, "exit"))) {
        return -1;
    } else {
        int iSendByte = 0;
        ZeroMemory(dnsSendBuff, sizeof(dnsSendBuff));
        pdnshdr->id = htons(0x0000);
        pdnshdr->flags = htons(0x0100);
        pdnshdr->questNum = htons(0x0001);
        pdnshdr->answerNum = htons(0x0000);
        pdnshdr->authorNum = htons(0x0000);
        pdnshdr->additionNum = htons(0x0000);

        memcpy(dnsSendBuff, pdnshdr, sizeof(DNSHDR));

        iSendByte += sizeof(DNSHDR);

        char * pTrace = hostname;
        char * pHostName = hostname;
        int iStrLen = strlen(hostname);
        u_char iCharNum = 0;
        while (*pTrace != '\0') {
            pTrace++;
        }

        while (pTrace != hostname) {
            *(pTrace + 1) = *pTrace;
            pTrace --;
        }
        *(pTrace + 1) = *pTrace;
        pTrace ++;
        while (*pTrace != '\0') {
            if (*pTrace == '.') {
                *pHostName = iCharNum;
                iCharNum = 0;
                pHostName = pTrace;
            } else {
                iCharNum ++;
            }
            pTrace ++;
        }

        *pHostName = iCharNum;

        memcpy(dnsSendBuff+ sizeof(DNSHDR), hostname, iStrLen + 2);

        iSendByte += (iStrLen +2);
        pqueryhddr->type = htons(0x0001);
        pqueryhddr->queryclass = htons(0x0001);
        memcpy(dnsSendBuff+ sizeof(DNSHDR)+iStrLen+2, pqueryhddr, sizeof(QUERYHDR));
        iSendByte += sizeof(QUERYHDR);
        return iSendByte;
    }
}

void decodeDNSPacket(char * DNSServBuff) {
    PDNSHDR  pdnshdr = (PDNSHDR) DNSServBuff;
    int iQueryNum, iRespNum, iAuthRespNum, iAdditionNum;
    iQueryNum = ntohs(pdnshdr->questNum);
    iRespNum = ntohs(pdnshdr->answerNum);
    iAuthRespNum = ntohs(pdnshdr->additionNum);
    if (pdnshdr->flags >> 15) {
        if ((pdnshdr->flags & 0x0007) == 3) {
            cout<<"No corresponding domain name entry."<<endl<<endl;
            return;
        }
        if ( (pdnshdr->flags >>10)  & 0x0001) {
            cout<<"Authoritative answer: "<<endl;
        } else {
            cout<<"None-authoritative answer: "<<endl;
        }

        char * pTraceResponse;
        pTraceResponse = DNSServBuff + sizeof(DNSHDR);

        while (*pTraceResponse) {
            pTraceResponse++;
        }
        pTraceResponse += sizeof(long);
        in_addr address;
        PRESPONSE  presponse ;
        cout<<"Address: ";
        for (int i=1; i<=iRespNum; i++) {
            pTraceResponse += sizeof(short);

            presponse = (PRESPONSE) pTraceResponse;
            if (ntohs(presponse->type) ==1) {
                pTraceResponse += sizeof(RESOPNSE);
                u_long ulIP = *(u_long *) pTraceResponse;
                address.s_addr = ulIP;
                if (i == iRespNum) {
                    cout<<inet_ntoa(address)<<'.';
                } else {
                    cout<<inet_ntoa(address)<<';';
                }
                pTraceResponse += sizeof(long);
            } else if (ntohs(presponse->type) == 5) {
                pTraceResponse += sizeof(RESOPNSE);
                pTraceResponse += ntohs(presponse->type);
            }
        }
        cout<<endl<<endl;

    } else {
        cout<<"Invalid DNS resolution!"<<endl<<endl;
    }
}

void getDnsServer(char * dnsServer) {
    DWORD nLength = 0;
    if (GetNetworkParams(NULL, &nLength) != ERROR_BUFFER_OVERFLOW) {
        return;
    }
    FIXED_INFO * pFixedInfo = (FIXED_INFO*) new BYTE[nLength];
    if (GetNetworkParams(pFixedInfo, &nLength) != ERROR_SUCCESS) {
        delete [] pFixedInfo;
        return;
    }

    IP_ADDR_STRING* pCurrentDnsServer = &pFixedInfo->DnsServerList;
    if (pCurrentDnsServer != NULL) {
        char* tmp = pCurrentDnsServer->IpAddress.String;
        strcpy(dnsServer, tmp);
    }
}

int  doDNS() {

    sockaddr_in addr ; //绑定地址
    SOCKET ListenSocket ; //发送与接收用的SOCKET
    int len = 0 ;

    loadWinsock();

    ListenSocket = socket(PF_INET , SOCK_DGRAM , 0) ;
    if(ListenSocket == INVALID_SOCKET) {
        printf("Error : socket create failed ! \n") ;
        fflush(0) ;
        return 0 ;
    }

    addr.sin_family = PF_INET ;
    addr.sin_addr.s_addr = htonl(INADDR_ANY) ; //任何地址
    addr.sin_port = htons(PORT) ;
    //进行监听端口的绑定
    if(bind(ListenSocket , (struct sockaddr*)&addr , sizeof(addr)) != 0)
    {
        printf("Error : bind failed !\n") ;
        fflush(0) ;
        closesocket(ListenSocket) ;
        return 0 ;
    }
    char dnsip[20] ;
    getDnsServer(dnsip) ;
    //进行UDP数据报的发送
    int sent ;
    hostent *hostdata ;

    cout<<dnsip;
    if(atoi(dnsip)) {    //是否为IP地址的标准形式
        u_long ip = inet_addr(dnsip) ;
        hostdata = gethostbyaddr((char*)&ip , sizeof(ip) , PF_INET) ;
        cout<<"atoi"<<endl;
    } else {
        printf("The DNS IP is not correct ! \n") ;
        return 0 ;
    }
    if(!hostdata) {
        printf("Get the name error ! \n") ;
        fflush(0) ;
        return 0 ;
    }


    sockaddr_in dest ; //填写目的地址信息
    dest.sin_family = PF_INET ;
    //将hostent结构体里面的h_addr_list转化为in_addr类型的地址信息
    dest.sin_addr = *(in_addr*)(hostdata->h_addr_list[0]) ;
    dest.sin_port = htons(PORT) ;

    //获取DNS报文
    char hostname[30] , buffer[100] ;
    DNSHDR dnsHdr ;
    QUERYHDR queryHdr ;
    printf("Please input domain name (no more than 30) : ") ;
    scanf("%s" , hostname) ;
    len = genDNSPack(&dnsHdr , &queryHdr , hostname , buffer) ;
    sent = sendto(ListenSocket , buffer , len , 0 , (sockaddr*)&dest , sizeof(sockaddr_in)) ;
    if(sent != len) {
        printf("Error : send error !\n") ;
        fflush(0) ;
        return 0 ;
    }

    //进行数据的接收
    char buf[400] ;
    sockaddr_in dnsServer ;
    int addr_len = sizeof(dnsServer) ;
    int result ;
    while(1) { //接收到数据时退出
        result = recvfrom(ListenSocket , buf , sizeof(buf) - 1 , 0 , (sockaddr*)&dnsServer , &addr_len) ;
        if(result > 0)
        {
            buf[result] = 0 ;
            decodeDNSPacket(buf) ;
            break ;
        }
    }

    return 0 ;
}

#endif //NETCPP_NDS_SERVER_H
