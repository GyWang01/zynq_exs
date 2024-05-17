#ifndef _GYRO_H
#define _GYRO_H

#define I2C_DEV "/dev/i2c-0"
#define GYRO_ADDR 0x68 // 陀螺仪I2C地址
#define ACCEL_XOUT_H 0x3B // 加速度X轴高字节寄存器地址
#define ACCEL_YOUT_H 0x3D // 加速度Y轴高字节寄存器地址
#define SCREEN_WIDTH 272 // 屏宽
#define SCREEN_HEIGHT 480 // 屏高

int setup_uinput_device();
void send_mouse_location_event(int fd, int x, int y);
void read_mpu6050_data(int , int *, int *);

#endif