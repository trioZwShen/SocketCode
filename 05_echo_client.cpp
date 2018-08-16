/*
 * 反射模型-客户端
 */

#include <stdio.h>
#include <unistd.h>     // read, write
#include <arpa/inet.h>  // 字节序
#include <netinet/in.h> // 地址结构
#include <sys/socket.h> // socket
#include <string.h>
#include "common_line.h"

int main()
{
    // 创建套接字
    printf("client init: socket\n");
    int fd = socket(PF_INET, SOCK_STREAM, 0);
    if (fd==-1){
        exit_own("套接字创建失败");
    }

    // 创建目标主机IP:PORT
    struct sockaddr_in svr_addr;
    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = PF_INET;
    svr_addr.sin_port = htons(4231);
    inet_aton("127.0.0.1", &svr_addr.sin_addr);

    // connect
    printf("client init: connect\n");
    if(connect(fd, (struct sockaddr*)&svr_addr, sizeof(svr_addr))==-1){
        exit_own("connext failed");
    }

    char buff[BUFFER_SIZE];
    while(true){
        memset(buff, 0, BUFFER_SIZE);
        int count = read(STDIN_FILENO, buff, BUFFER_SIZE);
        
        write(fd, buff, count);
        if (0==strncmp(buff, "exit", 4)){
            printf("disconnet\n");
            break;
        }
        memset(buff, 0, BUFFER_SIZE);
        count = read(fd, buff, BUFFER_SIZE);
        printf("echo ");
        write(STDOUT_FILENO, buff, count);
    }
    close(fd);
    return 0;
}

