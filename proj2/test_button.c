#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>

int main(int argc, char **argv)
{   
    int i, count = 2;
    char buffer[10];
    int fds[2];
    char *buttons[] = {"/dev/button0", "/dev/button1"};

    //打开
    for (i = 0; i < count; i++)
    {
        fds[i] = open(buttons[i], O_RDONLY);
        if (fds[i] < 0) {
            perror("Failed to open device");
            return -1;
        }
    }
    
    //读取
    for (i = 0; i < count; i++)
    {
        read(fds[i], buffer, 10);
        buffer[9] = '\0';
        printf("button%d status : %s\n", i, buffer);
    }
    
    //关闭
        for (i = 0; i < count; i++)
    {
        close(fds[i]);
    }

    printf("completed!");
    return 0;
}
