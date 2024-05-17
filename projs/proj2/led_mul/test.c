#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

// Usage eg: ./test /dev/led0 1

int main(int argc, char **argv) 
{
    char buffer[10];

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <device> <cmd>\n", argv[0]);
        return -1;
    }

    int fd = open(argv[1], O_RDWR);
    if (fd < 0) {
        perror("Failed to open device");
        return -1;
    }

    int cmd = atoi(argv[2]);  // 将命令参数转换为整数

    // 控制LED
    if (ioctl(fd, cmd, NULL) < 0) {
        perror("Failed to control LED");
        close(fd);
        return -1;
    }
    
    //read
    read(fd, buffer, 10);
    buffer[9] = '\0';
	printf("led status : %s\n", buffer);

    close(fd);
    return 0;
}
