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






int is_read_timeout(int fd, unsigned timeout){
    if (timeout==0){
        return 0;
    }

    int ret;
    fd_set fset;        
    FD_ZERO(&fset);     
    FD_SET(fd, &fset);      // init a fd_set
    
    struct timeval time;    // init a timeval
    time.tv_sec = timeout;
    time.tv_usec = 0;

    do{
        ret = select(fd+1, &fset, NULL, NULL, &time);
    }while(ret<0 && errno==EINTR);       // 如果select返回值小于0, 且errno为EINTR, 表示被信号中断

    if (ret==0){                        // timeout
        errno = ETIMEDOUT;              // set errno ETIMEDOUT
        return -1;
    }
    if (ret==1){                        // 没有超时
        return 0;
    }
    return ret;                         // 其余错误情况, 直接返回
}

int is_write_timeout(int fd, unsigned timeout){
    if (timeout==0){
        return 0;
    }

    int ret;
    fd_set fset;        
    FD_ZERO(&fset);     
    FD_SET(fd, &fset);      // init a fd_set
    
    struct timeval time;    // init a timeval
    time.tv_sec = timeout;
    time.tv_usec = 0;

    do{
        ret = select(fd+1, NULL, &fset, NULL, &time);
    }while(ret<0 && errno==EINTR);       // 如果select返回值小于0, 且errno为EINTR, 表示被信号中断

    if (ret==0){                        // timeout
        errno = ETIMEDOUT;              // set errno ETIMEDOUT
        return -1;
    }
    if (ret==1){                        // 没有超时
        return 0;
    }
    return ret;                         // 其余错误情况, 直接返回
}


int accept_timeout(int socket, struct sockaddr * addrets, socklen_t * address_len, unsigned timeout){
    if (timeout>0){
        fd_set fset;
        FD_ZERO(&fset);
        FD_SET(socket, &fset);

        struct timeval time;
        time.tv_sec = timeout;
        time.tv_usec = 0;
        
        int ret;
        do{
            ret = select(socket+1, &fset, NULL, NULL, &time);
        }while(ret<0 && errno == EINTR);

        if (ret==0){
            errno = ETIMEDOUT;
            return -1;
        }
        if (ret<0){
            return ret;
        }
    }

    int ret;
    if (addrets != NULL){
        ret = accept(socket, addrets, address_len);
    }else{
        ret = accept(socket, NULL, NULL);
    }
    return ret;
}

void set_block(int fd, bool block){
    int flag = fcntl(fd, F_GETFL);  // 获取这个文件描述符的flag
    if (flag == -1){
        exit_own("fcntl get error");
    }
    if (block){
        flag |= O_NONBLOCK;
    }else{
        flag &= ~O_NONBLOCK;
    }
    int ret = fcntl(fd, F_SETFL, flag);     // 设置文件描述符
    if(ret == -1){
        exit_own("fcntl set error");
    }
}


int connect_timeout(int socket, const struct sockaddr *address, socklen_t address_len, unsigned timeout){
    if (timeout>0){
        set_block(socket, false);
    }
    int ret = connect(socket, address, address_len);       // 非阻塞立刻返回
    if (ret<0 && errno == EINPROGRESS){                    // conn<0: connect失败, EINPROGRESS: 连接正在处理
        fd_set fset;
        FD_ZERO(&fset);
        FD_SET(socket, &fset);
        struct timeval time;
        time.tv_sec = timeout;
        time.tv_usec = 0;
        do {
            /* 一旦建立连接, 就可以进行写操作, 所以将其加入写集合 */
            ret = select(socket+1, NULL, &fset, NULL, &time);
        }while(ret<0 && errno == EINTR);
        
        if (ret==0){
            errno = ETIMEDOUT;
            return -1;
        }else if(ret<-1){
            return ret;

        }else if(ret==1){
            /** 
             * ret==1, 表示select认为socket已经准备好, 可能是由于确实准备好, 
             * 也可能是由于出现待处理错误, 如果是后者, 那么需要使用getsockopt来获取(不保存到errno)
             * */
            int err;
            socklen_t err_len = sizeof(err);
            int optret = getsockopt(socket, SOL_SOCKET, SO_ERROR, &err, &err_len);
            if (optret==-1){
                exit_own("getsockopt err error");
            }

            if(err!=0){
                errno = err;
                return -1;
            }else{          // 不是因为出现待处理错误引起的
                return 0;
            }
        }
    }

    if (timeout>0){
        set_block(socket, true);
    }
    return ret;
}
