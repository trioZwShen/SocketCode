/*
 * 回射模型-服务端
 */

#include <stdio.h>
#include <arpa/inet.h>      // 字节序转换函数
#include <netinet/in.h>     // 地址转换函数及地址结构
#include <sys/socket.h>     // socket函数
#include <unistd.h>
#include <string.h>         // 字符串处理函数
#include "common_line.h"

void do_process(int fd, struct sockaddr_in& client_addr){
    char buff[BUFFER_SIZE];
    while (true){
        memset(buff, 0, BUFFER_SIZE);
        int count = read(fd, buff, BUFFER_SIZE);
        if (count==0){
            printf("client %s: %d disconnect\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            break;
        }else if(count<0){
            exit_own("read error");
        }
        write(STDOUT_FILENO, buff, count);
        write(fd, buff, count);
    }
}

int main()
{
    // 创建监听套接字
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1){
        exit_own("监听套接字创建失败");
    }

    // 创建本机IP:Port地址结构
    struct sockaddr_in svr_addr;
    memset(&svr_addr, 0, sizeof(svr_addr));
    svr_addr.sin_family = PF_INET;
    svr_addr.sin_port = htons(4231);
    inet_aton("127.0.0.1", &svr_addr.sin_addr);
   
    // 开启SO_REUSEADDR
    int on=1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // 绑定监听套接字与地址结构
    if(bind(listen_fd, (struct sockaddr *)&svr_addr, sizeof(svr_addr))==-1){
        exit_own("绑定失败");
    }

    // listen监听套接字
    if(listen(listen_fd, SOMAXCONN)==-1){
        exit_own("监听失败");
    }

    


    while(true){
    	// 为建立链接的任务分配 已连接套接字
        sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t addr_len = sizeof(client_addr);
        int fd = accept(listen_fd, (struct sockaddr *)&client_addr, &addr_len);
        if (fd==-1){
            exit_own("accept failed");
        }
        printf("connected: %s: %u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        pid_t pid = fork();         // fork子进程
        if(pid==-1){                // fork failed
            exit_own("fork failed");
        }else if (pid==0){          // 子进程通信
            close(listen_fd);
            do_process(fd, client_addr);
            exit(0);
        }else{                      // 父进程监听
            close(fd);
        }
    }
    return 0;
}
