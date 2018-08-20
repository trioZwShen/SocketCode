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
#include <fcntl.h>      // fcntl


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



/*
 * 超时函数的封装, is_read_timeout, is_write_timeout, accept_timeout, connect_timeout
 */

/**
 *  判断读,写是否会超时
 *  不包含读写操作
 *  参数: fd文件描述符, timeout超时时间, 如果timeout==0, 表示不设置超时时间
 *  返回值: 返回0表示不超时, 返回-1 && errno == ETIMEDOUT 表示超时
 *          返回-1 && errno!=ETIMEDOUT 表示出错
 */
int is_read_timeout(int fd, unsigned timeout=0);
int is_write_timeout(int fd, unsigned timeout=0);


/**
 *  判断accept是否超时, 并执行accept
 *  参数: 
 *  @socket     : 套接字
 *  @address    : 客户端的地址
 *  @address_len: 地址长度
 *  @timeout    : 等待时长
 *  返回值: -1 && errno == ETIMEDOUT 表示超时
 *          -1 && errno != ETIMEDOUT 表示其他错误
 *          >=0 表示已连接套接字
 */
int accept_timeout(int socket, struct sockaddr * address, 
        socklen_t * address_len, unsigned timeout = 0);


/**
 *  设置文件描述符为阻塞/非阻塞
 */
void set_block(int fd, bool block = true);


/**
 *  connect_timeout
 *  如果connect能够连接上, 那么将会返回0
 *  如果connect超时还未连接上, 那么将会返回-1, 并设置errno为ETIMEDOUT
 *  如果connect出错, 那么将会返回-1
 */
int connect_timeout(int socket, const struct sockaddr *address, socklen_t address_len, unsigned timeout = 0);

#endif
