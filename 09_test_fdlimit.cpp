/**
 *  通过getrlimit获取一个进程能打开的最大文件描述符限制
 *  通过setrlimit设置一个进程能打开的最大文件描述符限制
 */

#include <stdio.h>
#include <sys/resource.h>
#include "common_line.h"

int main()
{
    struct rlimit li;
    if (getrlimit(RLIMIT_NOFILE, &li)<0){
        exit_own("getrlimit error");
    }
    printf("%lld, %lld\n", li.rlim_cur, li.rlim_max);

    li.rlim_cur = 1024;
    li.rlim_max = 2048;
    if (setrlimit(RLIMIT_NOFILE, &li)<0){
        exit_own("setrlimt error");
    }

    if (getrlimit(RLIMIT_NOFILE, &li)<0){
        exit_own("getrlimit error");
    }
    printf("%lld, %lld\n", li.rlim_cur, li.rlim_max);
    li.rlim_cur = 1024;
    li.rlim_max = 4096;
    if (setrlimit(RLIMIT_NOFILE, &li)<0){
        exit_own("setrlimt error");
    }

    if (getrlimit(RLIMIT_NOFILE, &li)<0){
        exit_own("getrlimit error");
    }
    printf("%lld, %lld\n", li.rlim_cur, li.rlim_max);
    return 0;
}
