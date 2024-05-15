#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <linux/i2c-dev.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include "gyro.h"

int setup_uinput_device() 
{
    struct uinput_setup usetup;
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if (fd < 0) 
    {
        perror("Opening /dev/uinput");
        return -1;
    }

    // 启用鼠标设备
    ioctl(fd, UI_SET_EVBIT, EV_REL);
    ioctl(fd, UI_SET_RELBIT, REL_X);
    ioctl(fd, UI_SET_RELBIT, REL_Y);
    ioctl(fd, UI_SET_EVBIT, EV_KEY);
    ioctl(fd, UI_SET_KEYBIT, BTN_LEFT);
    ioctl(fd, UI_SET_KEYBIT, BTN_RIGHT);

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x1234;  // 供应商ID
    usetup.id.product = 0x5678; // 产品ID
    strcpy(usetup.name, "Virtual Mouse Device");

    ioctl(fd, UI_DEV_SETUP, &usetup);
    ioctl(fd, UI_DEV_CREATE);

    sleep(1); // 等待设备被系统创建和设置
    return fd;
}

void send_mouse_location_event(int fd, int x, int y) 
{
    struct input_event ev;

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_REL;
    ev.code = REL_X;
    ev.value = x;
    write(fd, &ev, sizeof(struct input_event));

    ev.code = REL_Y;
    ev.value = y;
    write(fd, &ev, sizeof(struct input_event));

    ev.type = EV_SYN;
    ev.code = SYN_REPORT;
    ev.value = 0;
    write(fd, &ev, sizeof(struct input_event));
}

void read_mpu6000_data(int i2c_fd, int *x, int *y) 
{
    uint8_t buf[6];
    int16_t ax, ay;

    // 初始化陀螺仪
    // ioctl(i2c_fd, I2C_SLAVE, GYRO_ADDR);

    // 读取加速度数据
    buf[0] = ACCEL_XOUT_H;
    write(i2c_fd, buf, 1);
    read(i2c_fd, buf, 2);
    ax = (buf[0] << 8) | buf[1];
    buf[0] = ACCEL_YOUT_H;
    write(i2c_fd, buf, 1);
    read(i2c_fd, buf, 2);
    ay = (buf[0] << 8) | buf[1];

    *x = (SCREEN_WIDTH / 2) + ((float)ax / 16384.0 * 10); // 放大10倍
    *y = (SCREEN_HEIGHT / 2) - ((float)ay / 16384.0 * 10); // 放大10倍
}