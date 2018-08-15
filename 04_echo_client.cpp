/*
 * 反射模型-客户端
 * 考虑粘包问题, 通过自封packet解决
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

    packet buff;
    while(true){
        memset(&buff, 0, SSIZE_T_SIZE + BUFFER_SIZE);
        buff.len = read(STDIN_FILENO, buff.buffer, BUFFER_SIZE);    // write info to buff.buffer and get the lenght to buff.len
        writen(fd, &buff, SSIZE_T_SIZE + buff.len);                 // send buff to server

        memset(&buff, 0, SSIZE_T_SIZE + BUFFER_SIZE);
        int ret = readn(fd, &buff, SSIZE_T_SIZE);                   // receive buff.len first
        if (ret==-1){
            exit_own("readn error");
        }else if(ret<SSIZE_T_SIZE){
            printf("server colse\n");
            break;
        }

        ret = readn(fd, buff.buffer, buff.len);                     // receive buff.buffer bt buff.len
        if (ret==-1){
            exit_own("readn error");
        }else if(ret<buff.len){
            printf("server colse\n");
            break;
        }
        
        write(STDOUT_FILENO, buff.buffer, buff.len);                // write to STDOUT

    }
    close(fd);
    return 0;
}

