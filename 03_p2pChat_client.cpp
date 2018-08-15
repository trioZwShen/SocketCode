/*
 * 点对点聊天-客户端
 */

#include <stdio.h>
#include <unistd.h>         // fork
#include <sys/socket.h>     // socket
#include <arpa/inet.h>      // byte-order
#include <netinet/in.h>     // socket addr struct
#include <signal.h>         // signal
#include <string.h>         // memset
#include "common_line.h"

void handler(int sig){
    printf("sig = %d: send process close\n", sig);
    exit(0);
}

int main()
{
    int talk_fd = socket(PF_INET, SOCK_STREAM, 0);      // create communication socket
    if (talk_fd<0){
        exit_own("socket failed");
    }

    struct sockaddr_in server_addr;                     // create server addr
    server_addr.sin_family = PF_INET;
    server_addr.sin_port   = htons(58000);
    if (inet_aton("127.0.0.1", &server_addr.sin_addr)<0){
        exit_own("illegality ip");
    }

    if (connect(talk_fd, (struct sockaddr*)&server_addr, sizeof(server_addr))<0){
        exit_own("connect failed");
    }

    pid_t pid = fork();
    if (pid<0){
        exit_own("fork failed");

    }else if(pid>0){                                   // parent process, receive info
        char recvBuff[BUFFER_SIZE];
        while(true){
            memset(recvBuff, 0, BUFFER_SIZE);
            int recvCount = read(talk_fd, recvBuff, BUFFER_SIZE);
            if (recvCount<0){
                exit_own("read error");
            }else if(recvCount == 0){
                printf("receive close\n");
                break;
            }else{
                write(STDOUT_FILENO, recvBuff, recvCount);
            }
        }
        close(talk_fd);
        kill(pid, SIGHUP);                              // send signal SIGHUP to pid process
        exit(0);

    }else{                                              // child process, send info
        signal(SIGHUP, handler);
        char sendBuff[BUFFER_SIZE];
        while (true){
            memset(sendBuff, 0, BUFFER_SIZE);
            int sendCount = read(STDIN_FILENO, sendBuff, BUFFER_SIZE);
            if (sendCount<0){
                exit_own("read error");
            }else if (sendCount>0){
                write(talk_fd, sendBuff, sendCount);
            }
        }
    }
    return 0;
}
