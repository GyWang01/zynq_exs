#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>

int main(int argc, char **argv) {
    char *leds[] = {"/dev/led0", "/dev/led1", "/dev/led2"};
    int count = 3;
    int fd[100], i;

    // open leds
    for (i = 0; i < count; i++) {
        fd[i] = open(leds[i], O_RDWR);
        if (fd[i] < 0) {
            perror("Failed to open device");
            return -1;
        }
    }
    printf("Open LEDs success.\n");

    // flow leds
    char on = '1', off = '0';
    while (1) {
        for (i = 0; i < count; i++) {
            write(fd[i], &on, 1);  // Turn on
            sleep(1);
            write(fd[i], &off, 1); // Turn off
        }
    }
    return 0;
}
