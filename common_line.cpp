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


ssize_t recv_peek(int sockfd, void *buf, size_t len){
    while(true){
        int ret = recv(sockfd, buf, len, MSG_PEEK);
        if (ret==-1 && errno == EINTR)              // errno为EINTR被中断时不认为是出错,继续recv
            continue;
        return ret;
    }
}


ssize_t read_line(int sockfd, void * buf, size_t max_count){
    size_t nleft = max_count;
    char * tmpBuff = (char *)buf;
    while(nleft>0){
        ssize_t nread = recv_peek(sockfd, tmpBuff, nleft);  // peek
        if (nread<=0){
            return nread;
        }

        if (nread>nleft){
            exit_own("read_line error");
        }

        for (int i=0; i<nread; ++i){                        // 检查是否偷窥到换行
            if (tmpBuff[i]=='\n'){
                ssize_t ret = readn(sockfd, tmpBuff, i+1);  // 利用readn将这一行读取
                if(ret<i+1){
                    exit_own("read_line error");
                }
                return ret;
            }
        }
        
        // 如果本次读取没有换行
        nleft -= nread;
        ssize_t ret = readn(sockfd, tmpBuff, nread);
        if (ret<nread){
            exit_own("read_line error");
        }
        tmpBuff += nread;
    }
    return -1;
}

int get_localip(char * ip){
    char host[100] = {0};
    if (gethostname(host, sizeof(host))<0){
        return -1;
    }

    struct hostent *hp;
    if ((hp=gethostbyname(host))==NULL){
        return -1;
    }

    strcpy(ip, inet_ntoa(*(struct in_addr *)hp->h_addr_list[0]));       // 这个列表中的第一条就是本机的默认ip
    return 0;
}
