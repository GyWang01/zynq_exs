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
#include <signal.h> 
#include <stdlib.h> 

int uinput_fd;
int i2c_fd;

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

void cleanup(int signum) {
    // 清理操作
    ioctl(uinput_fd, UI_DEV_DESTROY);
    close(uinput_fd);
    close(i2c_fd);

    // 取消导出GPIO
    char path[40];
    int fd = open("/sys/class/gpio/unexport", O_WRONLY);
    if (fd == -1) {
        perror("Unexport GPIO");
    } 
	else 
	{
        snprintf(path, sizeof(path), "%d", GPIO_BTN_LEFT);
        write(fd, path, strlen(path));
        snprintf(path, sizeof(path), "%d", GPIO_BTN_RIGHT);
        write(fd, path, strlen(path));
        close(fd);
    }

    printf("Cleanup done\n");
    exit(0);
}

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
    // 注册SIGINT信号处理程序
    signal(SIGINT, cleanup);
	signal(SIGTERM, cleanup);
	
    int x, y;
    while (1) 
    {
        // 读取MPU6050的X和Y坐标
        read_mpu6050_data(i2c_fd, &x, &y);
        // 向虚拟设备发送鼠标移动事件
        send_mouse_location_event(uinput_fd, x, y);
        // 检测并向虚拟设备发送鼠标按键事件
        test_send_button(uinput_fd);
        usleep(10000);
    }

    return 0;
}


