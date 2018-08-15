/*
 * 字节序转换函数与地址转换函数
 */

#include <iostream>
#include <arpa/inet.h>  // 字节序转换函数
#include <netinet/in.h> // 地址转换函数以及地址结构体

int main()
{
    unsigned int x = 0x12345678;
    unsigned char * p = (unsigned char*)&x;
    
    printf("本机字节序: 0x%x, 0x%x, 0x%x, 0x%x\n", *p, *(p+1), *(p+2), *(p+3));

    x = htonl(x);
    p = (unsigned char*)&x;
    printf("网络字节序: 0x%x, 0x%x, 0x%x, 0x%x\n", *p, *(p+1), *(p+2), *(p+3));

    char ip[] = "192.168.10.154";
    struct in_addr netIp1;
    netIp1.s_addr = inet_addr(ip); 
    printf("地址转换函数1: %s\n", inet_ntoa(netIp1));

    struct in_addr netIp2;
    if (0==inet_aton(ip, &netIp2)){
        printf("非法IP\n");
        exit(-1);
    }
    printf("地址转换函数2: %s\n", inet_ntoa(netIp2));
    return 0;

}
