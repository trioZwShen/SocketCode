/*
 * 公共内敛函数
 * 公共宏
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>     // read  write
#include <errno.h>      // errno

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
