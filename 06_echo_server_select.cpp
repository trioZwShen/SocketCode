/*
 * 回射模型-服务端-使用select单进程来处理并发
 */

#include <stdio.h>
#include <arpa/inet.h>      // 字节序转换函数
#include <netinet/in.h>     // 地址转换函数及地址结构
#include <sys/socket.h>     // socket函数
#include <unistd.h>
#include <string.h>         // 字符串处理函数
#include <sys/wait.h>       // wait函数
#include <sys/select.h>     // select
#include "common_line.h"

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
    fd_set read_fd_set;             // care fd_set
    FD_ZERO(&read_fd_set);          // init
    int maxfd = listen_fd;
    FD_SET(listen_fd, &read_fd_set);
    int talk_fdVec[FD_SETSIZE];     // talk_fd array 
    for (int i=0; i<FD_SETSIZE; ++i){
        talk_fdVec[i] = -1;
    }

    while (true){
        fd_set tmp_set = read_fd_set;
        int ready_num = select(maxfd+1, &tmp_set, NULL, NULL, NULL);
        if (ready_num==-1){
            exit_own("select failed");
        }else if(ready_num == 0){
            printf("no client prepare\n");
            continue;
        }

        struct sockaddr_in client_addr;
        // 判断监听套接字是否有连接请求
        if (FD_ISSET(listen_fd, &tmp_set)){
            socklen_t client_addr_len = sizeof(client_addr); 
            int conn = accept(listen_fd, (struct sockaddr*)&client_addr, &client_addr_len);
            if (conn==-1){
                exit_own("accept failed");
            }

            int i;
            for (i=0; i<FD_SETSIZE; ++i){
                if (talk_fdVec[i] == -1){
                    talk_fdVec[i] = conn;
                    break;
                }
            }
            if (i==FD_SETSIZE){
                printf("\n");
                exit_own("too many clients");
            }
            printf("connected: %s: %u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

            // 已连接套接字加入描述符集合
            FD_SET(conn, &read_fd_set);
            if (maxfd<conn){        // 更新最大描述符
                maxfd = conn;
            }
            if (--ready_num<=0)
                continue;
        }

        // 已连接套接字
        for (int i=0; i<FD_SETSIZE; ++i){
            if(talk_fdVec[i]==-1){
                continue;
            }
            if (FD_ISSET(talk_fdVec[i], &tmp_set)){
                char buff[BUFFER_SIZE] = {0};
                int ret = read_line(talk_fdVec[i], buff, BUFFER_SIZE);
                if(ret==-1){
                    exit_own("read_line falied");

                }else if(ret==0){
                    printf("client close\n");
                    FD_CLR(talk_fdVec[i], &read_fd_set);
                    close(talk_fdVec[i]);
                    talk_fdVec[i] = -1;
                }
                printf("%s: %u ", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                fflush(stdout);
                writen(STDOUT_FILENO, buff, ret);
                writen(talk_fdVec[i], buff, ret);

                if (--ready_num<=0){
                    break;
                }
            }
        }
    }
    return 0;
}
