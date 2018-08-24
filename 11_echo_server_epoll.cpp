/**
 * 使用epoll实现回射服务器, 单进程高并发
 *
 */

#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <vector>
#include "common_line.h"

int main()
{
    // create listen fd
    int listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    if (listen_fd<0){
        exit_own("listen fd create failed");
    }

    // create server IP:Port
    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    server_addr.sin_port   = htons(4231);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int on = 1;
    int ret = setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));
    if (ret<0){
        exit_own("set failed");
    } 

    // bind fd with addr
    ret = bind(listen_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret<0){
        exit_own("bind failed");
    }

    // listen
    ret = listen(listen_fd, SOMAXCONN);
    if (ret<0){
        exit_own("listen failed");
    }
    
    // create epollfd object
    int epollfd = epoll_create(EPOLL_CLOEXEC);
    if (epollfd<0){
        exit_own("epoll_create failed");
    }

    // create a listen event
    struct epoll_event listen_event;
    listen_event.events = EPOLLIN | EPOLLET;
    listen_event.data.fd = listen_fd;

    // attach care fd to epollfd
    ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_fd, &listen_event);
    if (ret<0){
        exit_own("epoll_ctl add failed");
    }

    // create a event vector
    std::vector<epoll_event> eventVec(16);

    while(true){
        int event_count = epoll_wait(epollfd, &*eventVec.begin(), static_cast<int>(eventVec.size()), -1);
        if (event_count<0){
            if (errno ==EINTR){
                continue;
            }
            exit_own("epoll_wait failed");
        }else if (event_count==0){
            exit_own("epoll_wait timeout");
        }

        if (event_count==eventVec.size()){
            eventVec.resize(eventVec.size()*2);
        }

        for (int i=0; i<event_count; ++i){
            // listen fd prepare
            if (eventVec[i].data.fd == listen_fd){
                struct sockaddr_in client_addr;
                socklen_t client_len = sizeof(client_addr);
                int conn = accept(listen_fd, (struct sockaddr *)&client_addr, &client_len);
                if (conn<0){
                    exit_own("accpet failed");
                }
                printf("client accept: %s, %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
                
                // set nonblock and attach event
                set_block(conn, false);
                struct epoll_event accepted_event;
                accepted_event.events = EPOLLIN | EPOLLET;
                accepted_event.data.fd = conn;
                ret = epoll_ctl(epollfd, EPOLL_CTL_ADD, conn, &accepted_event);
                if (ret<0){
                    exit_own("epoll_ctl add failed");
                }
             
            // accepted fd prepare
            }else if(eventVec[i].events & EPOLLIN){
                int conn = eventVec[i].data.fd;
                if (conn<0)
                    continue;
                char buff[BUFFER_SIZE]={0};
                ret = read_line(conn, buff, BUFFER_SIZE);
                if (ret<0){
                    exit_own("read_line failed");
                }
                if (ret==0){
                    printf("client closed\n");
                    epoll_ctl(epollfd, EPOLL_CTL_DEL, conn, &eventVec[i]);
                    close(conn);
                    continue;
                }
                int filefd = fileno(stdout);   
                // put to stdout
                write(filefd, buff, ret);
                // echo
                writen(conn, buff, ret);
            }
        }
    }
    return 0;
}
