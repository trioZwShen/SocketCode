/*
 * 公共内敛函数
 * 公共宏
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // read  write
#include <errno.h>      // errno
#include <sys/socket.h> // recv, send
#include <netdb.h>      // gethostname, gethostbyname
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string.h>

#ifndef COMMON_LINE_H_
#define COMMON_LINE_H_

#define BUFFER_SIZE 1024
#define SSIZE_T_SIZE 8

inline void exit_own(const char * str, int code = -1){
    perror(str);
    exit(-1);
}

struct packet{
    size_t len;
    char buffer[BUFFER_SIZE];
};

/*
 * writen与readn, 作用域套接字, 读取目标长度的数据
 */
ssize_t writen(int fd, void * buffer, size_t count);
ssize_t readn(int fd, void * buffer, size_t count);


/*
 * recv_peek函数, 偷窥
 */
ssize_t recv_peek(int sockfd, void *buf, size_t len);


/*
 * read_line函数, 使用recv_peek
 */
ssize_t read_line(int sockfd, void * buf, size_t max_count);


/*
 * get_localip函数, 返回本机默认ip
 */
int get_localip(char * ip);


#endif
