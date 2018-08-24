/*
 * 反射模型-客户端-使用poll来优化
 */

#include <stdio.h>
#include <unistd.h>     // read, write
#include <arpa/inet.h>  // 字节序
#include <netinet/in.h> // 地址结构
#include <sys/socket.h> // socket
#include <string.h>
#include <poll.h>
#include "common_line.h"

void do_client(int fd){
    char recvbuff[BUFFER_SIZE] = {0};
    char sendbuff[BUFFER_SIZE] = {0};
    
    int file_in = fileno(stdin);        // transform filestream to fd
                                        // STDIN_FILENO 和 file_in是一样的, 但是当标准输入被重定向之后就不能用前者了

    struct pollfd fdVec[2];
    fdVec[0].fd = fd;
    fdVec[1].fd = file_in;
    fdVec[0].events = POLLIN;
    fdVec[1].events = POLLIN;
    int max = fd<file_in?file_in:fd;

    while(true){
        int ret = poll(fdVec, max+1, -1);
        if (ret==-1){
            exit_own("poll failed");
        }else if(ret==0){
            printf("there is no fd prepared\n");
            continue;
        }

        if (fdVec[0].revents & POLLIN){             // socket
            memset(recvbuff, 0, BUFFER_SIZE);
            int count = read_line(fdVec[0].fd, recvbuff, BUFFER_SIZE);
            if (count==-1){
                exit_own("read_line error");
            }else if(count==0){
                printf("server close\n");
                break;
            }
            write(STDOUT_FILENO, "echo ", 5);
            write(STDOUT_FILENO, recvbuff, count);
        }
        
        if (fdVec[1].revents & POLLIN){       // stdin
            memset(sendbuff, 0, BUFFER_SIZE);
            int count = read(fdVec[1].fd, sendbuff, BUFFER_SIZE);
            writen(fdVec[0].fd, sendbuff, count);
        }
    }
    close(fd);
}

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
    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);
    if (getsockname(fd, (struct sockaddr*)&local_addr, &addr_len)<0){
        exit_own("getsockname failed");
    }
    printf("local addr %s: %d\n", inet_ntoa(local_addr.sin_addr),ntohs(local_addr.sin_port));
    
    do_client(fd);
    
    return 0;
}
