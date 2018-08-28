/**
 *  udp聊天室的服务端
 *  功能: 维护在线人数列表,下发用户详情至客户端
 */

#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <string>
#include <iterator>
#include "head.h"
#include "../common_line.h"

USER_MAP online_user;

void do_login(int sock_fd, CMD_MSG & msg, struct sockaddr_in & client_addr){
    USER_INFO tmp_user;
    memcpy(&tmp_user, &msg.body, sizeof(USER_INFO));
    // complete the user info
    tmp_user.ip = inet_ntoa(client_addr.sin_addr);
    tmp_user.port = ntohs(client_addr.sin_port);
    
    if (online_user.count(tmp_user.name)){ /* login failed */
        CMD_MSG rsp_msg;
        rsp_msg.cmd = S2C_LOGIN_NACK;
        memcpy(&rsp_msg.body, &tmp_user, sizeof(USER_INFO));
        sendto(sock_fd, &rsp_msg, sizeof(CMD_MSG), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
        printf("[%s] is already login\n", tmp_user.name.c_str());

    }else{ /* login success */ 
        // login ack
        CMD_MSG rsp_msg;
        memset(&rsp_msg, 0, sizeof(CMD_MSG));
        rsp_msg.cmd = S2C_LOGIN_ACK;
        memcpy(&rsp_msg.body, &tmp_user, sizeof(USER_INFO));
        int ret = sendto(sock_fd, &rsp_msg, sizeof(CMD_MSG), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
        printf("[%s] %s: %d login\n", tmp_user.name.c_str(), tmp_user.ip.c_str(), tmp_user.port);
        
        // create rsp_msg
        memset(&rsp_msg, 0, sizeof(CMD_MSG));
        rsp_msg.cmd = S2C_SOMEONE_LOGIN;
        memcpy(&rsp_msg.body, &tmp_user, sizeof(USER_INFO));
        
        // notify
        USER_MAP::iterator it = online_user.begin();
        while(it != online_user.end()){
            // create notify target addr
            struct sockaddr_in tar_addr;
            tar_addr.sin_family = PF_INET;
            tar_addr.sin_port   = htons(it->second.port);
            inet_aton(it->second.ip.c_str(), &(tar_addr.sin_addr));     
            
            // notify
            sendto(sock_fd, &rsp_msg, sizeof(CMD_MSG), 0, (struct sockaddr*)&tar_addr, sizeof(tar_addr));
            printf("notify [%s] success\n", it->second.name.c_str());
            it++;
        }
        // update online user
        online_user.insert(std::pair<std::string, USER_INFO>(tmp_user.name, tmp_user));
    }
}

void do_logout(int sock_fd, CMD_MSG & msg, struct sockaddr_in & client_addr){
    USER_INFO tmp_user;
    memcpy(&tmp_user, &msg.body, sizeof(USER_INFO));
    
    // logout ack
    CMD_MSG rsp_msg;
    memset(&rsp_msg, 0, sizeof(CMD_MSG));
    rsp_msg.cmd = S2C_LOGOUT_ACK;
    sendto(sock_fd, &rsp_msg, sizeof(CMD_MSG), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
    printf("[%s] logout success\n", tmp_user.name.c_str());
    
    // delete the account;
    online_user.erase(tmp_user.name);
 
    // create rsp_msg
    memset(&rsp_msg, 0, sizeof(CMD_MSG));
    rsp_msg.cmd = S2C_SOMEONE_LOGOUT;
    memcpy(&rsp_msg.body, &tmp_user, sizeof(USER_INFO));

    // notify
    USER_MAP::iterator it = online_user.begin();
    while(it != online_user.end()){
        // create notify target addr
        struct sockaddr_in tar_addr;
        tar_addr.sin_family = PF_INET;
        tar_addr.sin_port   = htons(it->second.port);
        inet_aton(it->second.ip.c_str(), &(tar_addr.sin_addr));     
        
        // notify
        sendto(sock_fd, &rsp_msg, sizeof(CMD_MSG), 0, (struct sockaddr*)&tar_addr, sizeof(tar_addr));
        printf("notify [%s] success\n", it->second.name.c_str());
        it++;
    }
}

void do_syn(int sock_fd, CMD_MSG & msg, struct sockaddr_in & client_addr){
    printf("send online user to client\n");
    CMD_MSG rsp_msg;
    rsp_msg.cmd = S2C_ONLINE_USER_ACK;
    sendto(sock_fd, &rsp_msg, sizeof(CMD_MSG), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
    
    int num = online_user.size();
    sendto(sock_fd, &num, sizeof(num), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
    USER_MAP::iterator it;
    for (it = online_user.begin(); it!=online_user.end(); it++){
        std::string info = it->second.name + " " + it->second.ip +": "+ std::to_string(it->second.port)+"\n";
        sendto(sock_fd, info.c_str(), info.size(), 0, (struct sockaddr*)&client_addr, sizeof(client_addr));
    }
}

int main()
{
    int sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (sock_fd<0){
        exit_own("create socket failed");
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = PF_INET;
    server_addr.sin_port   = htons(5188);
    inet_aton("127.0.0.1", &server_addr.sin_addr);

    int ret = bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if (ret<0){
        exit_own("bind failed");
    }
    
    CMD_MSG msg;
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    while(true){
        memset(&msg, 0, sizeof(CMD_MSG));
        printf("%s\n", std::string(20, '-').c_str());
        int count = recvfrom(sock_fd, &msg, sizeof(CMD_MSG), 0, (struct sockaddr*)&client_addr, &client_addr_len);
        if (count<0){
            if (errno == EINTR)
                continue;
            exit_own("recvfrom error");
        }
        switch (msg.cmd){
            case C2S_LOGIN:{
                do_login(sock_fd, msg, client_addr);
                break;
            }
            case C2S_LOGOUT:{
                do_logout(sock_fd, msg, client_addr);
                break;
            }
            case C2S_ONLINE_USER:{
                do_syn(sock_fd, msg, client_addr);
                break;
            }
            default:{
                printf("!!!\n");
            }
        }
    }
    close(sock_fd);
    return 0;
}
