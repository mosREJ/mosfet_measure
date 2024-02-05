#include "cmds.h"
#include <stdio.h>
#include <unistd.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>
#include <time.h>

#define buf_size 256

int scpi_cmd(int fd, const char* cmd_format, ...)
{
    va_list args;
    va_start(args, cmd_format); 

    time_t curr_time = time(NULL);
    struct tm time = *localtime(&curr_time);

    printf("t: %2d:%2d:%2d\n", time.tm_hour, time.tm_min, time.tm_sec);
    
    char cmdbuf[buf_size];


#ifdef tee_problem
    setvbuf(stdout, cmdbuf, _IONBF, buf_size);
#endif
    vsnprintf(cmdbuf, buf_size, cmd_format, args);
    
    if(write(fd, cmdbuf, strlen(cmdbuf)) != strlen(cmdbuf)) 
        return -errno;

    puts(cmdbuf);

    va_end(args);
    
    return 0;
}

