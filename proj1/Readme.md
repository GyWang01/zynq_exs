# 用户空间体感鼠标

## uinput虚拟输入模块

uinput虚拟输入设备



## 鼠标坐标模块

打开i2c设备i2c_fd=open("/dev/i2c-0", O_RDWR)

配置i2c设备为陀螺仪ioctl(i2c_fd, I2C_SLAVE, GYRO_ADDR)

写入加速度X轴高字节寄存器地址，然后才能读回来并整合

```c
buf[0] = ACCEL_XOUT_H;
write(i2c_fd, buf, 1);
read(i2c_fd, buf, 2);
ax = (buf[0] << 8) | buf[1];
```

```c
*x = (SCREEN_WIDTH / 2) + ((float)ax / 16384.0 * 10); 
*y = (SCREEN_HEIGHT / 2) - ((float)ay / 16384.0 * 10);
```

- `ax` 和 `ay` 是原始的 16 位加速度值，范围为 `-32768` 到 `32767`。加速度值通常需要除以传感器的灵敏度范围（例如 16384，对于 ±2g 量程）来转换成标准单位（g）。

- 转换后的加速度值乘以 10 进行放大，使得转换后的值更易于观察和使用。

## gpio按键模块（使用gpio子系统）

首先导出gpio子系统```fd = open("/sys/class/gpio/export", O_WRONLY);```

然后向/sys/class/gpio/export写入个gpio号，初始化每个gpio

```c
snprintf(path, sizeof(path), "%d", gpio);	//将path格式化为对应的值
write(fd, path, strlen(path)
```

下一步是配置gpio为输入

```c
snprintf(path, sizeof(path), "/sys/class/gpio/gpio%d/direction", gpio);		////将path格式化为对应的路径
fd = open(path, O_WRONLY);
```

配置完成，后面就可以正常从/sys/class/gpio/gpio%d/value读值

## 发送事件模块

事件结构体input_event

```c
memset(&ev, 0, sizeof(struct input_event));
ev.type = EV_KEY;
ev.code = button;
ev.value = value;
write(fd, &ev, sizeof(struct input_event));
```

