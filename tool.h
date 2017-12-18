//
// Created by xiang on 2017/12/18.
//

#ifndef NETCPP_TOOL_H
#define NETCPP_TOOL_H

#include <winsock2.h>

bool loadWinsock() {
    WSADATA wsadata;
    return WSAStartup(MAKEWORD(2, 2), &wsadata) != 0;
}

#endif //NETCPP_TOOL_H
