#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: %s /dev/input/eventX\n", argv[0]);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("Failed to open input device");
        return 1;
    }

    struct input_event ev;
    while (1) {
        read(fd, &ev, sizeof(struct input_event));
        if (ev.type == EV_REL) {
            if (ev.code == REL_X) {
                printf("Mouse moved: X %d\n", ev.value);
            } else if (ev.code == REL_Y) {
                printf("Mouse moved: Y %d\n", ev.value);
            }
        } else if (ev.type == EV_KEY) {
            if (ev.code == BTN_LEFT && ev.value == 1) {
                printf("Left button pressed\n");
            } else if (ev.code == BTN_LEFT && ev.value == 0) {
                printf("Left button released\n");
            }
        }
    }

    close(fd);
    return 0;
}

