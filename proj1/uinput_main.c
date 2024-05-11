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
#include "button.h"

int main() 
{
    // 创建虚拟设备
    int uinput_fd = setup_uinput_device();
    if (uinput_fd == -1) 
    {
        fprintf(stderr, "Failed to setup uinput device.\n");
        return -1;
    }

    // 设置GPIO
    if (setup_gpio(GPIO_BTN_LEFT) == -1 || setup_gpio(GPIO_BTN_RIGHT) == -1) {
        fprintf(stderr, "Failed to setup GPIOs.\n");
        return -1;
    }

    // 打开I2C设备
    int i2c_fd = open(I2C_DEV, O_RDWR);
    if (i2c_fd < 0) 
    {
        perror("Opening I2C device");
        close(uinput_fd);
        return -1;
    }
    // 配置陀螺仪设备
    if (ioctl(i2c_fd, I2C_SLAVE, GYRO_ADDR) < 0) 
    {
        perror("Setting I2C device as slave");
        close(i2c_fd);
        close(uinput_fd);
        return -1;
    }
    int x, y;

    while (1) 
    {
        // 读取MPU6000的X和Y坐标
        read_mpu6000_data(i2c_fd, &x, &y);
        // 向虚拟设备发送鼠标移动事件
        send_mouse_location_event(uinput_fd, x, y);
        // 检测并向虚拟设备发送鼠标按键事件
        test_send_button(uinput_fd);
        usleep(10000);
    }

    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd);
    close(i2c_fd);
    return 0;
}