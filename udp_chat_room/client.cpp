/**
 * udp聊天室-客户端
 */

#include <stdio.h>
#include <iostream>
#include <vector>
#include <string>
#include <sys/socket.h>
#include <sys/select.h>
#include "head.h"
#include "../common_line.h"

std::string nick_name;

void do_command(int sock_fd){
    char buff[MSG_LEN] = {0};
    memset(buff, 0, MSG_LEN);
    int count = read(STDIN_FILENO, buff, MSG_LEN);
    if (count<0){
        exit_own("read error");
    }
    
    std::vector<std::string> command(3);
    int j = 0;
    for (int i = 0; i<count; ++i){
        if (buff[i]==' '){
            j++;
            continue;
        }
        if (buff[i]=='\n')
            continue;
        command[j] += buff[i];
    }

    CMD_MSG msg;
    memset(&msg, 0, sizeof(CMD_MSG));
    if (command[0] == "exit"){
        printf("Bye!\n");
        exit(0);

    }else if(command[0] == "login"){
        if (nick_name!=""){
            printf("please logout first\n");
            return;
        }
        if (command[1]==""){
            printf("illegel name\n");
            return;
        }
        msg.cmd = C2S_LOGIN;
        USER_INFO tmp_user;
        tmp_user.name = command[1];
        memcpy(&msg.body, &tmp_user, sizeof(USER_INFO));
        sendto(sock_fd, &msg, sizeof(CMD_MSG), 0, NULL, 0);

    }else if(command[0] == "logout"){
        if (nick_name==""){
            printf("please login first\n");
            return;
        }
        msg.cmd = C2S_LOGOUT;
        USER_INFO tmp_user;
        tmp_user.name = nick_name;
        memcpy(&msg.body, &tmp_user, sizeof(USER_INFO));
        sendto(sock_fd, &msg, sizeof(CMD_MSG), 0, NULL, 0);

    }else if(command[0] == "chat"){
        printf("chat\n");

    }else if(command[0] == "help"){
        printf("exit                - exit\n");
        printf("login {name}        - create a account and login\n");
        printf("logout              - delete a account and logout\n");
        printf("chat {name} {...}   - chat with someone\n");

    }else if(command[0] == "info"){
        msg.cmd = C2S_ONLINE_USER;
        sendto(sock_fd, &msg, sizeof(CMD_MSG), 0, NULL, 0);

    }else{
        printf("!!!\n");
    }
}

void do_S2C(int sock_fd){
    CMD_MSG recv_msg;
    memset(&recv_msg, 0, sizeof(CMD_MSG));

    int count = recvfrom(sock_fd, &recv_msg, sizeof(CMD_MSG), 0, NULL, NULL);
    if (count<0){
        if (errno == EINTR)
            return;
        exit_own("recvfrom failed");
    }

    if (recv_msg.cmd == S2C_LOGIN_NACK){
        USER_INFO tmp_user;
        memset(&tmp_user, 0, sizeof(USER_INFO));
        memcpy(&tmp_user, recv_msg.body, sizeof(USER_INFO));
        printf("[%s] is already login\n", tmp_user.name.c_str());

    }else if(recv_msg.cmd == S2C_LOGIN_ACK){
        USER_INFO tmp_user;
        memset(&tmp_user, 0, sizeof(USER_INFO));
        memcpy(&tmp_user, recv_msg.body, sizeof(USER_INFO));
        printf("[%s] login success\n", tmp_user.name.c_str());
        nick_name = tmp_user.name;

    }else if(recv_msg.cmd == S2C_LOGOUT_ACK){
        printf("[%s] logout success\n", nick_name.c_str());
        nick_name = "";

    }else if(recv_msg.cmd == S2C_SOMEONE_LOGIN){
        USER_INFO tmp_user;
        memset(&tmp_user, 0, sizeof(USER_INFO));
        memcpy(&tmp_user, recv_msg.body, sizeof(USER_INFO));
        printf("NOTIFY [%s] is online\n", tmp_user.name.c_str());

    }else if(recv_msg.cmd == S2C_SOMEONE_LOGOUT){
        USER_INFO tmp_user;
        memset(&tmp_user, 0, sizeof(USER_INFO));
        memcpy(&tmp_user, recv_msg.body, sizeof(USER_INFO));
        printf("NOTIFY [%s] is offline\n", tmp_user.name.c_str());

    }else if(recv_msg.cmd == S2C_ONLINE_USER_ACK){
        int num;
        int ret = recvfrom(sock_fd, &num, sizeof(num), 0, NULL, NULL);
        if (ret<0){
            if (errno == EINTR)
                return;
            exit_own("recvfrom failed");
        }
        printf("Online User Count %d\n", num);

        char buff[MSG_LEN] = {0};
        for (int i=0; i<num; ++i){
            memset(buff, 0, MSG_LEN);
            ret = recvfrom(sock_fd, buff, sizeof(buff), 0, NULL, NULL);
            if (ret<0){
                if (errno == EINTR)
                    return;
                exit_own("recvfrom failed");
            }
            write(STDOUT_FILENO, buff, ret);
        }
    }
}

void do_something(int sock_fd){
    fd_set read_set;
    FD_ZERO(&read_set);
    FD_SET(sock_fd, &read_set);
    int file_in = fileno(stdin);
    FD_SET(file_in, &read_set);
    int maxfd = sock_fd>file_in?sock_fd:file_in;

    while(true){
        // enter command
        printf("%s\n", std::string(20, '-').c_str());
        printf("Please enter command\n");

        fd_set tmp_set = read_set;
        int ret = select(maxfd+1, &tmp_set, NULL, NULL, NULL);
        if (ret<0){
            if (errno == EINTR)
                continue;
            exit_own("select error");
        }else if(ret==0){
            exit_own("select time out");
        }

        if (FD_ISSET(file_in, &tmp_set)){
            do_command(sock_fd);
        }
        if (FD_ISSET(sock_fd, &tmp_set)){
            do_S2C(sock_fd);
        }
    }
}

int main()
{
    int sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock_fd<0){
        exit_own("socket failed");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    server_addr.sin_port   = htons(5188);
    inet_aton("127.0.0.1", &server_addr.sin_addr);
    int ret = connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret<0){
        exit_own("connect failed");
    }

    do_something(sock_fd);
    return 0;
} 
