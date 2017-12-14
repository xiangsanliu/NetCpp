//
// Created by xiang on 2017/12/14.
//

#ifndef NETCPP_CRC_CHECK_H
#define NETCPP_CRC_CHECK_H

#include <iostream>
using namespace std;

string deleteZero(string a) {
    unsigned int index=0;
    for (int i=0 ;i<a.length(); i++) {
        if (a[index] == '1')
            break;
        index++;
    }
    return a.substr(index, a.length());
}

string calculate(string a, string b) {
    if (a.length()<b.length())
        return a;
    string result = b;
    for (int i=0; i<b.length(); i++) {
        result[i] = a[i]==b[i] ? '0':'1';
    }
    return deleteZero(result);
}

int doCRCCheck() {

    string source, divisor;
    int n;
    cout<<"input source data:"<<endl;
    cin>>source;
    cout<<"input divisor:"<<endl;
    cin>>divisor;
    cout<<"input n:"<<endl;
    cin>>n;
    int length = source.length() + n;
    string dividend = source;
    for (int i=0 ;i< n; i++) {
        dividend += '0';
    }
    unsigned int index = 0;
    string remainder;
    while (index < length) {
        int remainderLength = remainder.length();
        remainder += dividend.substr(index, divisor.length() - remainderLength);
        index += divisor.length()- remainderLength;
        remainder = calculate(remainder, divisor);
    }
    int numOfZero = n-remainder.length();
    string zero;
    for (int i=0 ;i<numOfZero; i++) {
        zero+='0';
    }
    remainder = zero + remainder;
    cout<<remainder;
    return 0;
}

#endif //NETCPP_CRC_CHECK_H
