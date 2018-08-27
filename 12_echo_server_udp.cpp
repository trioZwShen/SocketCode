/**
 * 基于UDP实现回射客户-服务模型
 */


#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include "common_line.h"



int main()
{
    int talk_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (talk_fd<0){
        exit_own("create socket failed");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    server_addr.sin_port = htons(4321);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int ret = bind(talk_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret<0){
        exit_own("bind failed");
    }

    char buff[BUFFER_SIZE];
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    while(true){
        memset(buff, 0, BUFFER_SIZE);
        int count = recvfrom(talk_fd, buff, BUFFER_SIZE, 0, (struct sockaddr *)&client_addr, &client_addr_len);
        if (count<0){
            if (errno == EINTR)
                continue;
            exit_own("recvfrom error");
        }
        if(count>0){
            printf("client %s %d: ", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            fflush(stdout);
            write(STDOUT_FILENO, buff, count);
            sendto(talk_fd, buff, count, 0, (struct sockaddr *)&client_addr, client_addr_len);
        }
    }
    close(talk_fd);
    return 0;
}
