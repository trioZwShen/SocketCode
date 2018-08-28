/**
 *  udp聊天室的头文件
 *
 */

#ifndef HEAD_H_
#define HEAD_H_

#include <map>
#include <string>

#define C2S_LOGIN           0x01
#define C2S_LOGOUT          0x02
#define C2S_ONLINE_USER     0x03

#define S2C_LOGIN_ACK       0x01
#define S2C_LOGIN_NACK      0x02
#define S2C_LOGOUT_ACK      0x03
#define S2C_SOMEONE_LOGIN   0x04
#define S2C_SOMEONE_LOGOUT  0x05
#define S2C_ONLINE_USER_ACK 0x06
#define C2C_CHAT            0x07

#define MSG_LEN             1024

typedef struct cmd_msg{
    int             cmd;
    char            body[MSG_LEN];
}CMD_MSG;

typedef struct chat_msg{
    std::string     name;
    char            body[MSG_LEN];
}CHAT_MSG;

typedef struct user_info{
    std::string     name;
    std::string     ip;
    unsigned short  port;
}USER_INFO;

typedef std::map<std::string, USER_INFO> USER_MAP;


#endif
