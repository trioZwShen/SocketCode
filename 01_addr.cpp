/*
 * @remark : 测试本机字节序
 */

#include <stdio.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main()
{
    unsigned int x = 0x12345678;
    unsigned char *p = (unsigned char *)&x;
    // 测试主机字节序
    printf("主机字节序: 0x%x, 0x%x, 0x%x, 0x%x\n", *p, *(p+1), *(p+2), *(p+3));

    // 转换为网络字节序
    unsigned int net_x = htonl(x);
    p = (unsigned char *)&net_x;
    printf("网络字节序: 0x%x, 0x%x, 0x%x, 0x%x\n", *p, *(p+1), *(p+2), *(p+3));
    
    // 地址转换函数
    char ip[]="192.168.1.1";
    struct in_addr netIp;
    int ret = inet_aton(ip, &netIp);
    if (0==ret){
        printf("IP地址非法\n");
        exit(-1);
    }
    printf("地址转换函数测试1: %s\n", inet_ntoa(netIp));
    
    unsigned int tmp = inet_addr(ip);
    netIp.s_addr = tmp;
    printf("地址转换函数测试2: %s\n", inet_ntoa(netIp));

    return 0;
}
