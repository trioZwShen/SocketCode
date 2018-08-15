/*
 * 公共内敛函数
 * 公共宏
 */

#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 1024

inline void exit_own(const char * str, int code = -1){
    perror(str);
    exit(-1);
}


