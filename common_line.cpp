/*
 * 公共函数的定义
 */

#include "common_line.h"

ssize_t writen(int fd, void * buffer, size_t count){
    ssize_t nleft = count;                                  // nleft bytes need to writen 
    char * tmpBuff = (char *)buffer;                        // buffer start
    while(nleft>0){                                         // loop if nleft>0
        ssize_t nwriten = write(fd, tmpBuff, nleft);        // write nleft bytes to fd
        if (nwriten<0){                                 
            if (errno == EINTR)
                continue;
            return -1;
        }else if(nwriten==0){
           continue;
        }
        nleft -= nwriten;                                   // update nleft and tmpBuff
        tmpBuff += nwriten;
    }
    return count;
}

ssize_t readn(int fd, void * buffer, size_t count){
    ssize_t nleft = count;
    char * tmpBuff = (char *)buffer;
    while(nleft>0){
        ssize_t nread = read(fd, tmpBuff, nleft);
        if (nread<0){
            if (errno == EINTR)         // 被信号中断了, 不认为出错
                continue;
            return -1;                  // 失败返回-1
        }else if(nread==0){
            return count-nleft;
        }
        nleft -= nread;
        tmpBuff += nread;
    }
    return count;
}
