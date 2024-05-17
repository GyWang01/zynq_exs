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

void read_mpu6050_data(int i2c_fd, int *x, int *y) 
{
    uint8_t buf[6];
    int16_t ax, ay;

    // 初始化陀螺仪
    ioctl(i2c_fd, I2C_SLAVE, GYRO_ADDR);

    // 读取加速度数据
    buf[0] = ACCEL_XOUT_H;
    write(i2c_fd, buf, 1);
    read(i2c_fd, buf, 2);
    ax = (buf[0] << 8) | buf[1];
    buf[0] = ACCEL_YOUT_H;
    write(i2c_fd, buf, 1);
    read(i2c_fd, buf, 2);
    ay = (buf[0] << 8) | buf[1];

    *x = -(float)ax / 16384.0 * 10; // 放大10倍
    *y = -(float)ay / 16384.0 * 10; // 放大10倍
}