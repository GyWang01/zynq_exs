#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/of_gpio.h>
#include <linux/gpio.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/delay.h>

struct mpu_mouse_data {
    struct i2c_client *client;
    unsigned int io_down;        // GPIO编号，用于设备控制或中断
    struct input_dev *input_dev; // 输入设备，用于生成鼠标事件
    struct delayed_work work;	//工作队列,用于处理数据读取
    unsigned int gpio_left;      
    unsigned int gpio_right;     
};

static int mpu_mouse_read_data(struct i2c_client *client, int16_t *ax, int16_t *ay) {
    int ret;
    uint8_t buf[2]; // 用于存储读取的高字节和低字节

    // 读取加速度X轴数据（高字节和低字节）
    ret = i2c_smbus_read_i2c_block_data(client, 0x3B, 2, buf);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read X-axis data\n");
        return ret;
    }
    *ax = (int16_t)((buf[0] << 8) | buf[1]); // 合并高字节和低字节

    // 读取加速度Y轴数据（高字节和低字节）
    ret = i2c_smbus_read_i2c_block_data(client, 0x3D, 2, buf);
    if (ret < 0) {
        dev_err(&client->dev, "Failed to read Y-axis data\n");
        return ret;
    }
    *ay = (int16_t)((buf[0] << 8) | buf[1]); // 合并高字节和低字节

    return 0;
}

static void mpu_mouse_report_event(struct mpu_mouse_data *mouse_data, int ax, int ay) {
    struct input_dev *input = mouse_data->input_dev;
    // 报告相对X/Y轴移动
    input_report_rel(input, REL_X, ax);
    input_report_rel(input, REL_Y, ay);
    input_sync(input); // 同步事件到输入子系统
}

static int mpu_mouse_remove(struct i2c_client *client) {
    struct mpu_mouse_data *mouse_data = i2c_get_clientdata(client);
    cancel_delayed_work_sync(&mouse_data->work); // 取消延迟工作
    input_unregister_device(mouse_data->input_dev);
	gpio_free(mouse_data->gpio_left);
    gpio_free(mouse_data->gpio_right);
    return 0;
}

static void mpu_mouse_work_handler(struct work_struct *work) {
    struct mpu_mouse_data *mouse_data = container_of(work, struct mpu_mouse_data, work.work);
    int16_t ax, ay;
    int left_state, right_state;

    left_state = gpio_get_value(mouse_data->gpio_left);
    right_state = gpio_get_value(mouse_data->gpio_right);

    if (mpu_mouse_read_data(mouse_data->client, &ax, &ay) == 0) {
        mpu_mouse_report_event(mouse_data, ax, ay);
    }

    // 报告左右键状态
    input_report_key(mouse_data->input_dev, BTN_LEFT, left_state);
    input_report_key(mouse_data->input_dev, BTN_RIGHT, right_state);
    input_sync(mouse_data->input_dev);

    schedule_delayed_work(&mouse_data->work, msecs_to_jiffies(50)); // 再次排定工作
}

static int mpu_mouse_probe(struct i2c_client *client, const struct i2c_device_id *id) {
    struct device_node *np = client->dev.of_node;    // 获取设备的设备树节点
    struct mpu_mouse_data *mydevice;
    int error, ret;
    // 为设备分配内存
    mydevice = devm_kzalloc(&client->dev, sizeof(*mydevice), GFP_KERNEL);
    if (!mydevice){
        dev_err(&client->dev, "nomem\n");
        return -ENOMEM;
    }
    mydevice->client = client;
    i2c_set_clientdata(client, mydevice);
    // 打印设备地址
    dev_alert(&client->dev, "MPU Address: 0x%02x\n", client->addr);
    // 从设备树中获取GPIO编号 
    mydevice->io_down = of_get_named_gpio(np, "io_down", 0);
    pr_err("gpio=%d\n",mydevice->io_down);
    // 检查I2C功能
    if (!i2c_check_functionality(client->adapter, I2C_FUNC_SMBUS_I2C_BLOCK)) {
        dev_err(&client->dev, "I2C check functionality failed.\n");
        return -ENXIO;
    }
    // 读取MPU6050的设备ID进行验证
    error = i2c_smbus_read_byte_data(client, 0x75);
    if(error != 0x68){
        dev_err(&client->dev, "id check failed.\n");
        return -ENXIO;
    } 
    dev_alert(&client->dev, "id=%x.\n",error);
    // 创建输入设备
    mydevice->input_dev = input_allocate_device();
    if (!mydevice->input_dev) {
        dev_err(&client->dev, "Failed to allocate input device\n");
        return -ENOMEM;
    }
    mydevice->input_dev->name = "MPU6050 Mouse";
    mydevice->input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
    mydevice->input_dev->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y);
    mydevice->input_dev->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT) | BIT_MASK(BTN_RIGHT);
	mydevice->gpio_left = of_get_named_gpio(np, "gpio-left", 0);
    mydevice->gpio_right = of_get_named_gpio(np, "gpio-right", 0);
	if (!gpio_is_valid(mydevice->gpio_left) || !gpio_is_valid(mydevice->gpio_right)) {
        dev_err(&client->dev, "Invalid GPIO\n");
        return -EINVAL;
    }
    gpio_request(mydevice->gpio_left, "gpio_left");
    gpio_direction_input(mydevice->gpio_left);
    gpio_request(mydevice->gpio_right, "gpio_right");
    gpio_direction_input(mydevice->gpio_right);
	// 注册输入设备
    ret = input_register_device(mydevice->input_dev);
    if (ret) {
        dev_err(&client->dev, "Failed to register input device\n");
        input_free_device(mydevice->input_dev);
        return ret;
    }
	INIT_DELAYED_WORK(&mydevice->work, mpu_mouse_work_handler);
    schedule_delayed_work(&mydevice->work, msecs_to_jiffies(50)); // 每50毫秒执行一次
    return 0;
    return 0;
}

// 定义与设备树匹配的设备列表
static const struct of_device_id mpu_of_match[] = {
    { .compatible = "inv,mpu6050" },
    { }
};
MODULE_DEVICE_TABLE(of, mpu_of_match);

// 定义I2C驱动结构体
static struct i2c_driver mpu_mouse_driver = {
    .probe = mpu_mouse_probe,
    .remove = mpu_mouse_remove,
    .driver = {
        .name = "inv-mpu6050",
        .owner = THIS_MODULE,
        .of_match_table = of_match_ptr(mpu_of_match),
    },
};

module_i2c_driver(mpu_mouse_driver);
MODULE_DESCRIPTION("MPU6050 mouse driver");
MODULE_LICENSE("GPL");
