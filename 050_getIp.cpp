/*
 * 使用gethostname()和gethostbyname
 */

#include <stdio.h>
#include <sys/socket.h>
#include "common_line.h"

int main()
{
    char ip[16] = {0};
    get_localip(ip);
    printf("%s\n", ip);
    return 0;
}
