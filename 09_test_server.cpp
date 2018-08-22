/**
 * 最大文件描述符测试-服务端
 */

#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common_line.h"


int main()
{
    // create listen socket
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd<0){
        exit_own("socket failed");
    }

    // init addr
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_port = htons(5800);
    server_addr.sin_family = PF_INET;
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    // setsockopt
    int on = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    // bindadrr
    int ret = bind(listen_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret<0){
        exit_own("bind failed");
    }
    int count = 1;
    while(1){
        printf("%d\n", count++);
        // listen socket
        ret = listen(listen_fd, SOMAXCONN);
        if (ret<0){
            exit_own("listen failed");
        }

        // accept_timeout
        struct sockaddr_in client_addr;
        memset(&client_addr, 0, sizeof(client_addr));
        socklen_t addr_len = sizeof(client_addr);
        int conn  = accept_timeout(listen_fd, (struct sockaddr*)&client_addr, &addr_len, 0);
        if (conn<0 && errno == ETIMEDOUT){
            exit_own("timeout");
        }else if(conn<0){
            exit_own("accept_timeout failed");
        }else{
            printf("accept success\n");
        }
    }
    return 0;
}
