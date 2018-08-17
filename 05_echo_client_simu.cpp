/*
 * 反射模型-客户端, 模拟多个并发断开的情况
 */

#include <stdio.h>
#include <unistd.h>     // read, write
#include <arpa/inet.h>  // 字节序
#include <netinet/in.h> // 地址结构
#include <sys/socket.h> // socket
#include <string.h>
#include "common_line.h"

void do_client(int fd){
    char buff[BUFFER_SIZE];
    while(true){
        memset(buff, 0, BUFFER_SIZE);
        int count = read(STDIN_FILENO, buff, BUFFER_SIZE);
        writen(fd, buff, count);

        memset(buff, 0, BUFFER_SIZE);
        count = read_line(fd, buff, BUFFER_SIZE);
        if (count==-1){
            exit_own("read_line error");
        }else if(count==0){
            printf("server close\n");
            break;
        }
        write(STDOUT_FILENO, "echo ", 5);
        write(STDOUT_FILENO, buff, count);
    }
    close(fd);
}

int main()
{
    // 创建套接字
    int fd[5];
    for (int i=0; i<5; ++i){
        
        fd[i] = socket(PF_INET, SOCK_STREAM, 0);
        if (fd[i]==-1){
            exit_own("套接字创建失败");
        }

        // 创建目标主机IP:PORT
        struct sockaddr_in svr_addr;
        memset(&svr_addr, 0, sizeof(svr_addr));
        svr_addr.sin_family = PF_INET;
        svr_addr.sin_port = htons(4231);
        inet_aton("127.0.0.1", &svr_addr.sin_addr);

        // connect
        if(connect(fd[i], (struct sockaddr*)&svr_addr, sizeof(svr_addr))==-1){
            exit_own("connext failed");
        }
        struct sockaddr_in local_addr;
        socklen_t addr_len = sizeof(local_addr);
        if (getsockname(fd[i], (struct sockaddr*)&local_addr, &addr_len)<0){
            exit_own("getsockname failed");
        }
        printf("local addr %s: %d\n", inet_ntoa(local_addr.sin_addr),ntohs(local_addr.sin_port));
        
    }
    do_client(fd[0]);
    return 0;
}
