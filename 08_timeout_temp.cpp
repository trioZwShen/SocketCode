#include <unistd.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include "common_line.h"

void test_read(){
    char buff[BUFFER_SIZE] = {0};
    int ret = is_read_timeout(STDIN_FILENO, 5);
    if (ret==0){
        ret = read(STDIN_FILENO, buff, BUFFER_SIZE);
        if (ret==-1){
            exit_own("read error");
        }
        write(STDOUT_FILENO, buff, ret);
    }else if(ret == -1){
        printf("timeout\n");
    }
}

int main()
{
    test_read();
    return 0;
}
