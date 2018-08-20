/**
 * 超时函数测试-客户端
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "common_line.h"

int main()
{
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    if (sock<0){
        exit_own("socket failed");
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_port = htons(5800);
    server_addr.sin_family = PF_INET;
    inet_aton("127.0.0.1", &server_addr.sin_addr);
    int ret = connect_timeout(sock, (struct sockaddr*)&server_addr, sizeof(server_addr), 5);
    if (ret<0){
        exit_own("connect failed");
    }else{
        printf("connet successed\n");
    }

    return 0;
}
