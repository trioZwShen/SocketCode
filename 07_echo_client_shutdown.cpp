/*
 * 反射模型-客户端-使用select来优化
 */

#include <stdio.h>
#include <unistd.h>     // read, write
#include <arpa/inet.h>  // 字节序
#include <netinet/in.h> // 地址结构
#include <sys/socket.h> // socket
#include <string.h>
#include <sys/select.h> // select
#include "common_line.h"

void do_client(int fd){
    char recvbuff[BUFFER_SIZE] = {0};
    char sendbuff[BUFFER_SIZE] = {0};
    
    fd_set read_fd_set;     // create a read_fd_set
    FD_ZERO(&read_fd_set);  // init
    
    int file_in = fileno(stdin);        // transform filestream to fd
    // STDIN_FILENO 和 file_in是一样的, 但是当标准输入被重定向之后就不能用前者了

    bool stdin_eof = false;
    while(true){
        FD_SET(fd, &read_fd_set);
        if (!stdin_eof)
            FD_SET(file_in, &read_fd_set);  // 每次都得重新加入

        int ret = select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL);
        if (ret==-1){
            exit_own("select failed");
        }else if(ret==0){
            printf("there is no fd prepared\n");
            continue;
        }

        if (FD_ISSET(fd, &read_fd_set)){             // socket
            memset(recvbuff, 0, BUFFER_SIZE);
            int count = read_line(fd, recvbuff, BUFFER_SIZE);
            if (count==-1){
                exit_own("read_line error");

            }else if(count==0){
                printf("server close\n");
                close(fd);
                break;

            }else{
                write(STDOUT_FILENO, "echo ", 5);
                write(STDOUT_FILENO, recvbuff, count);
            }
        }
        
        if (FD_ISSET(file_in, &read_fd_set)){       // stdin
            memset(sendbuff, 0, BUFFER_SIZE);
            int count = read(STDIN_FILENO, sendbuff, BUFFER_SIZE);
            if (count == 0){
                stdin_eof = true;
                shutdown(fd, SHUT_WR);
            }else{
                writen(fd, sendbuff, count);
            }
        }
    }
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
