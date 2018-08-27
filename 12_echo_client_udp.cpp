/**
 * 基于UDP实现简单的客户服务器回射模型
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/select.h>
#include "common_line.h"


int main()
{
    int talk_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (talk_fd<0){
        exit_own("socket error");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = ntohs(4321);
    inet_aton("127.0.0.1", &server_addr.sin_addr);
    socklen_t server_addr_len = sizeof(server_addr);

    int ret = connect(talk_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret<0){
        exit_own("connect falied");
    }

    fd_set read_fd_set;
    FD_ZERO(&read_fd_set);
    int file_in = fileno(stdin);
    FD_SET(talk_fd, &read_fd_set);
    FD_SET(file_in, &read_fd_set);
    int maxfd = talk_fd>file_in?talk_fd:file_in;

    char buff[BUFFER_SIZE] = {0};
    while(true){
        fd_set tmp_set = read_fd_set;
        int ret = select(maxfd+1, &tmp_set, NULL, NULL, NULL);
        if (ret<0){
            if (errno==EINTR)
                continue;
            exit_own("select error");
        }else if(ret==0){
            exit_own("select timeout");
        }

        if (FD_ISSET(talk_fd, &tmp_set)){
            memset(buff, 0, BUFFER_SIZE);
            int count = recvfrom(talk_fd, buff, BUFFER_SIZE, 0, NULL, NULL);
            if (count<0){
                if (errno == EINTR)
                    continue;
                exit_own("recvfrom error");
            }
            writen(STDOUT_FILENO, buff, count);
        }

        if (FD_ISSET(file_in, &tmp_set)){
            memset(buff, 0, BUFFER_SIZE);
            int count = read(file_in, buff, BUFFER_SIZE);
            if (count<0){
                exit_own("read error");
            }
            ret = sendto(talk_fd, buff, count, 0, NULL, NULL);
            if (ret<0){
                exit_own("sendto failed");
            }
        }
    }
    close(talk_fd);
    return 0;
}
