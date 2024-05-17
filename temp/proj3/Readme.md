# 内核空间体感鼠标驱动

该代码在内核空间配合使用**设备树**开发。涉及的驱动开发知识有

- i2c子系统
- gpio子系统
- 输入子系统
- 定时器和工作队列
- 设备树

`i2c_driver`结构体是核心：

```c
static struct i2c_driver mpu_mouse_driver = {
	.probe = mpu_mouse_probe,
	.remove = mpu_mouse_remove,
	.driver = {
		.name = "inv-mpu6050",
		.owner = THIS_MODULE,
		.of_match_table = of_match_ptr(mpu_of_match),
	},
};
```

在编译该驱动之前首先要修改设备树文件，提供mpu6050的硬件信息，在amba节点下添加设备：

```dts
mpu6050@68{
compatible = "inv,mpu6050";
reg = <0x68>;
io_down = <0x7 54 1>;
};
```

编译设备树文件并重启加载后，内核会自动创建`i2c_device`结构体

当insmod该驱动时，mpu_mouse_driver会自动匹配设备树`compatible = "inv,mpu6050"`的`i2c_device`结构体。匹配成功后会自动调用probe函数。

## probe函数

如前所述，加载驱动后会自动调用probe函数：

I2C 子系统定义了驱动程序与设备之间的接口。对于 I2C 驱动，`probe` 函数的参数类型必须是 `struct i2c_client` 和 `const struct i2c_device_id`。

即

```c
static int mpu_mouse_probe(struct i2c_client *client, const struct i2c_device_id *id)
```

