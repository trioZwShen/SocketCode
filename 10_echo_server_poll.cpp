/*
 * 回射模型-服务端-使用poll单进程来处理并发
 */

#include <stdio.h>
#include <arpa/inet.h>      // 字节序转换函数
#include <netinet/in.h>     // 地址转换函数及地址结构
#include <sys/socket.h>     // socket函数
#include <unistd.h>
#include <string.h>         // 字符串处理函数
#include <sys/wait.h>       // wait函数
#include <poll.h>           // poll
#include "common_line.h"
#define FD_VEC_SIZE 2048

void handle_sigchld(int sig){
    // -1 表示处理全部的子进程, NULL表示不关注退出状态, WNOHANG表示不挂起
    while(waitpid(-1, NULL, WNOHANG)>0)
        continue;
}

int main()
{
    // signal attach
    signal(SIGCHLD, handle_sigchld);

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
    
    // use select
    int maxfd = listen_fd;
    struct pollfd fdVec[FD_VEC_SIZE];
    for (int i=0; i<FD_VEC_SIZE; ++i){
        fdVec[i].fd = -1;
    }
    fdVec[0].fd = listen_fd;
    fdVec[0].events = POLLIN;

    while (true){
        int ready_num = poll(fdVec, maxfd+1, -1);
        if (ready_num==-1){
            exit_own("poll failed");
        }else if(ready_num == 0){
            printf("no client prepare\n");
            continue;
        }

        struct sockaddr_in client_addr;
        // 判断监听套接字是否有连接请求
        if (fdVec[0].revents & POLLIN){
            socklen_t client_addr_len = sizeof(client_addr); 
            int conn = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
            if (conn==-1){
                exit_own("accept failed");
            }

            int i;
            for (i=0; i<FD_VEC_SIZE; ++i){
                if (fdVec[i].fd == -1){
                    fdVec[i].fd = conn;
                    fdVec[i].events = POLLIN;
                    break;
                }
            }
            if (i==FD_VEC_SIZE){
                exit_own("too many clients");
            }
            printf("connected: %s: %u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // 已连接套接字加入描述符集合
            if (maxfd<conn){        // 更新最大描述符
                maxfd = conn;
            }
            if (--ready_num<=0)
                continue;
        }

        // 已连接套接字
        for (int i=0; i<=maxfd; ++i){
            if(fdVec[i].fd==-1){
                continue;
            }
            if (fdVec[i].revents & POLLIN){
                char buff[BUFFER_SIZE] = {0};
                int ret = read_line(fdVec[i].fd, buff, BUFFER_SIZE);
                if(ret==-1){
                    exit_own("read_line falied");

                }else if(ret==0){
                    printf("client close\n");
                    close(fdVec[i].fd);
                    fdVec[i].fd = -1;

                }else{
                    printf("%s: %u ", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                    fflush(stdout);
                    writen(STDOUT_FILENO, buff, ret);
                    writen(fdVec[i].fd, buff, ret);
                }
                if (--ready_num<=0){
                    break;
                }
            }
        }
    }
    return 0;
}
