# 内核空间的LED驱动

## file_operations结构体

实现open/read/write/release/ioctl都在这个结构体中。用户空间对于的系统调用会找到该结构体下的对应成员

```c
struct file_operations light_fops =
{
	.owner = THIS_MODULE,
	.unlocked_ioctl = light_ioctl,
	.open = light_open,
	.release = light_release,
    .read = light_read,
    .write = light_write,
};
```

接下来就是实现对应的成员

## light_open(struct inode *inode,struct file *filp)

从设备节点中获取次设备号,通过判断次设备号来确定lednum参数，并存入file结构体的私有数据，以供之后的read/write等函数使用

```c
iminor(inode);
filp->private_data = (void *) lednum;
```

使用Linux对gpio提供的底层函数来配置gpio及其方向

```c
result = gpio_request(lednum, "Leds");
result = gpio_direction_output(lednum, 1);//1为输出
```

## 其他file_operations成员

```light_release(struct inode *inode,struct file *filp)```

要调用`gpio_free(lednum);`释放资源

`light_ioctl(struct file *filp,unsigned int cmd, unsigned long arg)`

和`light_write`

要调用`gpio_set_value(lednum, 0);`设置gpio的值

`light_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos)`

要调用`gpio_get_value(lednum);`

## light_init(void)模块

```c
major = register_chrdev(0, "leds", &light_fops);
ledClass = class_create(THIS_MODULE, "myled_class");
for (i=0; i<LED_TOTAL; i++) {
    ledDevice = device_create(ledClass, NULL, MKDEV(major, i), 		NULL, "led%d", i);
```

`register_chrdev`,向内核注册字符设备，自动创建`cdev`结构体

`class_create`创建设备类，创建`/sys/class/myled_class`

`device_create`创建设备文件，即创建`/dev/led*`

当然在`light_cleanup`函数中要有对应的

```c
device_destroy(ledClass, MKDEV(major, i));
class_destroy(ledClass);
unregister_chrdev(major, "leds");
```

# 内核空间的按键驱动

代码逻辑几乎和LED驱动一样。因为都是调用由gpio控制，只是控制LED灯的gpio只能输出，控制按键的gpio只能输入