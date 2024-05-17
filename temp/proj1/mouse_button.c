#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include "button.h"


int setup_gpio(int gpio) 
{
    char path[40];
    int fd;

    // 导出GPIO
    fd = open("/sys/class/gpio/export", O_WRONLY);
    if (fd == -1) {
        perror("Export GPIO");
        return -1;
    }
    snprintf(path, sizeof(path), "%d", gpio);
	// 向/sys/class/gpio/export写入个gpio号，初始化gpio
    if (write(fd, path, strlen(path)) < 0) 
    {
        perror("Failed to export GPIO");
        close(fd);
        return -1;
    }
    close(fd);
    // 设置GPIO为输入
    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", gpio);
    fd = open(path, O_WRONLY);
    if (fd == -1) {
        perror("Open GPIO direction");
        return -1;
    }
    if (write(fd, "in", 2) < 0) {
        perror("Set GPIO direction");
        close(fd);
        return -1;
    }
    close(fd);

    return 0;
}

// 读取GPIO值
int read_gpio(int gpio) {
    char path[40], value_str[3];
    int fd, value;

    snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/value", gpio);
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Read GPIO value");
        return -1;
    }

    if (read(fd, value_str, 3) < 0) {
        perror("Failed to read GPIO value");
        close(fd);
        return -1;
    }

    close(fd);
    value = atoi(value_str);

    return value;
}

void send_mouse_button_event(int fd, int button, int value) 
{
    struct input_event ev;

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_KEY;
    ev.code = button;
    ev.value = value;
    write(fd, &ev, sizeof(struct input_event));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(struct input_event));
}

void test_send_button(int uinput_fd)
{
        // 检测并发送鼠标按键事件
        if (read_gpio(GPIO_BTN_LEFT) == 0) {
            send_mouse_button_event(uinput_fd, BTN_LEFT, 0); // 按下左键
            usleep(100000); // 等待100ms
            send_mouse_button_event(uinput_fd, BTN_LEFT, 1); // 松开左键
        }

        if (read_gpio(GPIO_BTN_RIGHT) == 0) {
            send_mouse_button_event(uinput_fd, BTN_RIGHT, 0); // 按下右键
            usleep(100000); // 等待100ms
            send_mouse_button_event(uinput_fd, BTN_RIGHT, 1); // 松开右键
        }
}